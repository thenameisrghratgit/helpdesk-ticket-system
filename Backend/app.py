from flask import Flask, request, jsonify, render_template
import subprocess
import os

BASE_DIR = os.path.dirname(os.path.abspath(__file__))

app = Flask(__name__, template_folder="Frontend")

SESSION = {"loggedIn": False, "username": "", "role": ""}

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

                tickets.append({
                    "id": p[0],
                    "username": p[1],          
                    "requesterName": p[2], 
                    "service": p[3],
                    "description": p[4],
                    "status": p[5],
                    "date": "Today"
                })
    except:
        pass

    return jsonify({"ok": True, "tickets": tickets})

# ---------- STATUS (QUEUE FEATURE 🔥) ----------
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
        return jsonify({
            "ok": True,
            "status": status,
            "peopleAhead": int(ahead)
        })

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

#------------delete---------
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