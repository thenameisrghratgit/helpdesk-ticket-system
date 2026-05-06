from flask import Flask, request, jsonify, render_template
import subprocess
import os

BASE_DIR = os.path.dirname(os.path.abspath(__file__))

app = Flask(__name__, template_folder="Frontend")

SESSION = {"loggedIn": False, "username": "", "role": ""}

# ── SERVICE MAPPING (number → name) ─────────────────────────────────
SERVICE_MAP = {
    1:  "Wi-Fi / Internet Issues",
    2:  "Electrical Issues",
    3:  "Air Conditioning / Ventilation",
    4:  "IT Support / Computer Issues",
    5:  "Projector / AV Equipment",
    6:  "Plumbing / Water Issues",
    7:  "Carpentry / Furniture Repair",
    8:  "Cleaning / Housekeeping",
    9:  "Laboratory Equipment",
    10: "Printer / Scanner Issues",
    11: "Access Card / Door Lock Issues",
    12: "Network Infrastructure",
    13: "Hostel / Accommodation",
    14: "Transport / Bus Service",
    15: "Library Services",
    16: "Software / License Issues",
    17: "Campus Infrastructure",
    18: "Other"
}

def service_string_to_num(service):
    """Mirror the C getServiceNum logic."""
    mapping = [
        ("Wi-Fi",            1),
        ("Electrical",       2),
        ("Air Conditioning", 3),
        ("IT Support",       4),
        ("Projector",        5),
        ("Plumbing",         6),
        ("Carpentry",        7),
        ("Cleaning",         8),
        ("Laboratory",       9),
        ("Printer",         10),
        ("Access",          11),
        ("Network",         12),
        ("Hostel",          13),
        ("Transport",       14),
        ("Library",         15),
        ("Software",        16),
        ("Campus",          17),
    ]
    for keyword, num in mapping:
        if keyword in service:
            return num
    return 18

# ---------- PAGES ----------
@app.route('/')
def home():
    return render_template("homepage.html")

@app.route('/login')
def login_page():
    return render_template("login.html")

@app.route('/open')
def open_ticket():
    return render_template("openticket.html")

@app.route('/status')
def status_page():
    return render_template("mystatus.html")

@app.route('/admin')
def admin_page():
    return render_template("admin.html")

# ---------- LOGIN ----------
@app.route('/api/login', methods=['POST'])
def login():
    data = request.get_json()
    res = subprocess.run(
        [os.path.join(BASE_DIR, "main.exe"), "login", data['username'], data['password']],
        capture_output=True, text=True, cwd=BASE_DIR
    )
    out = res.stdout.strip()
    if out == "ADMIN":
        SESSION.update({"loggedIn": True, "username": data['username'], "role": "admin"})
        return jsonify({"ok": True, "role": "admin"})
    if out == "USER":
        SESSION.update({"loggedIn": True, "username": data['username'], "role": "user"})
        return jsonify({"ok": True, "role": "user"})
    return jsonify({"ok": False, "message": "Invalid login"})

# ---------- REGISTER ----------
@app.route('/api/register', methods=['POST'])
def register():
    data = request.get_json()
    res = subprocess.run(
        [os.path.join(BASE_DIR, "main.exe"), "register", data['username'], data['password']],
        capture_output=True, text=True, cwd=BASE_DIR
    )
    if "EXISTS" in res.stdout:
        return jsonify({"ok": False, "message": "User exists"})
    return jsonify({"ok": True})

# ---------- SUBMIT ----------
@app.route('/api/ticket/submit', methods=['POST'])
def submit():
    if not SESSION["loggedIn"]:
        return jsonify({"ok": False, "message": "Not logged in"})
    data = request.get_json()
    res = subprocess.run(
        [os.path.join(BASE_DIR, "user.exe"),
         SESSION["username"],
         data['requesterName'],
         data['service'],
         data['description']],
        capture_output=True, text=True, cwd=BASE_DIR
    )
    tid = res.stdout.strip().split("|")[1]
    return jsonify({"ok": True, "message": f"Ticket #{tid} created"})

# ---------- LIST ----------
@app.route('/api/ticket/list')
def list_tickets():
    tickets = []
    try:
        with open("ticketgen.txt") as f:
            for line in f:
                p = line.strip().split("|")
                if SESSION["role"] != "admin" and p[1] != SESSION["username"]:
                    continue
                tickets.append({
                    "id": p[0],
                    "username": p[1],
                    "requesterName": p[2],
                    "service": p[3],
                    "serviceNum": service_string_to_num(p[3]),   # ← ADDED
                    "description": p[4],
                    "status": p[5],
                    "date": "Today"
                })
    except:
        pass
    return jsonify({"ok": True, "tickets": tickets})

# ── NEW: tickets grouped by service (BST order = numeric order) ──────
@app.route('/api/ticket/grouped')
def grouped_tickets():
    """Returns tickets grouped by service number, FIFO within each group."""
    groups = {}   # serviceNum → list of tickets (FIFO)
    try:
        with open("ticketgen.txt") as f:
            for line in f:
                p = line.strip().split("|")
                if len(p) < 6:
                    continue
                if SESSION["role"] != "admin" and p[1] != SESSION["username"]:
                    continue
                snum = service_string_to_num(p[3])
                ticket = {
                    "id": p[0],
                    "username": p[1],
                    "requesterName": p[2],
                    "service": p[3],
                    "serviceNum": snum,
                    "serviceName": SERVICE_MAP.get(snum, "Other"),
                    "description": p[4],
                    "status": p[5]
                }
                groups.setdefault(snum, []).append(ticket)
    except:
        pass

    # convert to sorted list of groups
    result = []
    for snum in sorted(groups.keys()):
        result.append({
            "serviceNum": snum,
            "serviceName": SERVICE_MAP.get(snum, "Other"),
            "tickets": groups[snum]   # already FIFO (file order preserved)
        })

    return jsonify({"ok": True, "groups": result})

# ── NEW: tickets for a specific service number ───────────────────────
@app.route('/api/ticket/by-service')
def tickets_by_service():
    snum = request.args.get("num", type=int)
    if snum is None:
        return jsonify({"ok": False, "message": "Missing service num"})

    tickets = []
    try:
        with open("ticketgen.txt") as f:
            for line in f:
                p = line.strip().split("|")
                if len(p) < 6:
                    continue
                if service_string_to_num(p[3]) != snum:
                    continue
                if SESSION["role"] != "admin" and p[1] != SESSION["username"]:
                    continue
                tickets.append({
                    "id": p[0],
                    "username": p[1],
                    "requesterName": p[2],
                    "service": p[3],
                    "serviceNum": snum,
                    "serviceName": SERVICE_MAP.get(snum, "Other"),
                    "description": p[4],
                    "status": p[5]
                })
    except:
        pass

    return jsonify({"ok": True, "tickets": tickets,
                    "serviceName": SERVICE_MAP.get(snum, "Other")})

# ── NEW: service number → name mapping ──────────────────────────────
@app.route('/api/services')
def services():
    return jsonify({"ok": True, "services": SERVICE_MAP})

# ---------- STATUS ----------
@app.route('/api/ticket/status')
def status():
    tid = request.args.get("id")
    res = subprocess.run(
        [os.path.join(BASE_DIR, "user.exe"), SESSION["username"], tid],
        capture_output=True, text=True, cwd=BASE_DIR
    )
    out = res.stdout.strip()
    if out.startswith("FOUND"):
        _, status, ahead = out.split("|")
        return jsonify({"ok": True, "status": status, "peopleAhead": int(ahead)})
    return jsonify({"ok": False})

# ---------- UPDATE ----------
@app.route('/api/ticket/update', methods=['POST'])
def update():
    data = request.get_json()
    subprocess.run(
        [os.path.join(BASE_DIR, "admin.exe"), data['ticketId'], data['status']],
        cwd=BASE_DIR
    )
    return jsonify({"ok": True})

# ---------- SESSION ----------
@app.route('/api/session')
def session():
    return jsonify(SESSION)

# ---------- LOGOUT ----------
@app.route('/api/logout', methods=['POST'])
def logout():
    SESSION.update({"loggedIn": False})
    return jsonify({"ok": True})

# ---------- DELETE ----------
@app.route('/api/ticket/delete', methods=['POST'])
def delete_ticket():
    data = request.get_json()
    tid = data['ticketId']
    lines = []
    with open("ticketgen.txt", "r") as f:
        lines = f.readlines()
    with open("ticketgen.txt", "w") as f:
        for line in lines:
            if not line.startswith(str(tid)):
                f.write(line)
    return jsonify({"ok": True})

if __name__ == '__main__':
    app.run(debug=True)
