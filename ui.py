import flet as ft
import socket
import json

SERVER_HOST = "127.0.0.1"
SERVER_PORT = 8080
BUFFER_SIZE = 10000


# ---------- TCP CLIENT ----------
def send_request(payload: dict):
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((SERVER_HOST, SERVER_PORT))
    s.send(json.dumps(payload).encode())
    data = s.recv(BUFFER_SIZE).decode()
    s.close()
    return json.loads(data)


# ---------- MAIN APP ----------
def main(page: ft.Page):
    page.title = "Omni Docs"
    page.theme_mode = "dark"
    page.padding = 20

    current_user = {"username": None}
    current_file = {"name": None}

    # ---------- LOGIN ----------
    username = ft.TextField(label="Username", width=300)
    password = ft.TextField(label="Password", password=True, width=300)
    login_status = ft.Text()

    def do_login(e):
        res = send_request({
            "operation": "login",
            "parameters": {
                "username": username.value,
                "password": password.value
            }
        })

        if res["status"] == "success":
            current_user["username"] = username.value
            load_home()
        else:
            login_status.value = "Invalid credentials"
            page.update()

    login_view = ft.Column(
        [
            ft.Text("Omni Docs", size=32, weight="bold"),
            username,
            password,
            ft.ElevatedButton("Login", on_click=do_login),
            login_status
        ],
        alignment="center",
        horizontal_alignment="center"
    )

    # ---------- HOME ----------
    search_box = ft.TextField(
        hint_text="Search files...",
        expand=True
    )

    files_grid = ft.GridView(
        expand=True,
        runs_count=4,
        spacing=20,
        run_spacing=20
    )

    def load_files(prefix=""):
        files_grid.controls.clear()

        # Create New File card
        files_grid.controls.append(
        ft.Container(
                content=ft.Column(
                    [
                        ft.Icon(ft.Icons.ADD, size=40),
                        ft.Text("Create New File")
                    ],
                    alignment="center",
                    horizontal_alignment="center",
                ),
                bgcolor=ft.Colors.BLUE_GREY_800,
                border_radius=10,
                padding=20,
                ink=True,  # 🔥 REQUIRED
                on_click=lambda e: create_file_dialog(),
            )
        )

        res = send_request({
            "operation": "get_file_names",
            "parameters": {
                "username": current_user["username"],
                "prefix": prefix
            }
        })

        if res["status"] == "success":
            for fname in res["data"]["file_names"]:
                files_grid.controls.append(
                    ft.Container(
                        content=ft.Column(
                            [
                                ft.Icon(ft.Icons.CODE, size=30),
                                ft.Text(fname)
                            ],
                            alignment="center",
                            horizontal_alignment="center"
                        ),
                        bgcolor=ft.Colors.BLUE_GREY_700,
                        border_radius=10,
                        padding=20,
                        on_click=lambda e, f=fname: open_editor(f)
                    )
                )

        page.update()

    search_box.on_change = lambda e: load_files(search_box.value)

    home_view = ft.Column([
        ft.Row([
            search_box,
            ft.IconButton(
                icon=ft.Icons.KEY,
                tooltip="Read Shared File",
                on_click=lambda e: read_shared_dialog()
            )
        ]),
        files_grid
    ])

    # ---------- EDITOR ----------
    code_editor = ft.TextField(multiline=True, expand=True)
    doc_view = ft.TextField(multiline=True, expand=True, read_only=True)

    def generate_doc(e):
        res = send_request({
            "operation": "generate_doc",
            "parameters": {
                "username": current_user["username"],
                "filename": current_file["name"]
            }
        })
        if res["status"] == "success":
            doc_view.value = res["data"]["encoded_data"]
            page.update()

    editor_view = ft.Column([
        ft.Row([
            ft.Text("", size=20),
            ft.IconButton(
                icon=ft.Icons.SHARE,
                tooltip="Share File",
                on_click=lambda e: share_file()
            ),
            ft.ElevatedButton("Generate Documentation", on_click=generate_doc)
        ]),
        ft.Row([
            code_editor,
            doc_view
        ], expand=True)
    ], expand=True)

    # ---------- ACTIONS ----------
    def open_editor(fname):
        current_file["name"] = fname

        res = send_request({
            "operation": "read_file",
            "parameters": {
                "username": current_user["username"],
                "filename": fname
            }
        })

        if res["status"] == "success":
            code_editor.value = res["data"]["result_data"]

        page.controls.clear()
        page.add(editor_view)
        page.update()

    def create_file_dialog():
        fname = ft.TextField(label="File name", autofocus=True)
        content = ft.TextField(
            label="Initial content",
            multiline=True,
            min_lines=6,
        )
    
        def create(e):
            send_request({
                "operation": "create_file",
                "parameters": {
                    "username": current_user["username"],
                    "filename": fname.value,
                    "data": content.value
                }
            })
            dlg.open = False
            page.update()
            load_home()
    
        dlg = ft.AlertDialog(
            modal=True,
            title=ft.Text("Create New File"),
            content=ft.Container(
                width=400,
                content=ft.Column(
                    [fname, content],
                    tight=True,
                ),
            ),
            actions=[
                ft.TextButton("Cancel", on_click=lambda e: close_dialog()),
                ft.ElevatedButton("Create", on_click=create),
            ],
            actions_alignment=ft.MainAxisAlignment.END,
        )
    
        def close_dialog():
            dlg.open = False
            page.update()
    
        page.overlay.append(dlg)   # 🔥 REQUIRED
        dlg.open = True
        page.update()


    def share_file():
        target = ft.TextField(label="Share with user")

        def share(e):
            res = send_request({
                "operation": "share_file",
                "parameters": {
                    "owner": current_user["username"],
                    "filename": current_file["name"],
                    "target_user": target.value
                }
            })
            page.snack_bar = ft.SnackBar(ft.Text("Share Key: " + res["data"]["result_data"]))
            page.snack_bar.open = True
            page.update()

        page.dialog = ft.AlertDialog(
            title=ft.Text("Share File"),
            content=target,
            actions=[ft.TextButton("Share", on_click=share)]
        )
        page.dialog.open = True
        page.update()

    def read_shared_dialog():
        key = ft.TextField(label="Enter share key")

        def read(e):
            res = send_request({
                "operation": "read_shared_file",
                "parameters": {
                    "username": current_user["username"],
                    "key": key.value
                }
            })
            if res["status"] == "success":
                code_editor.value = res["data"]["result_data"]
                page.controls.clear()
                page.add(editor_view)
            page.update()

        page.dialog = ft.AlertDialog(
            title=ft.Text("Read Shared File"),
            content=key,
            actions=[ft.TextButton("Read", on_click=read)]
        )
        page.dialog.open = True
        page.update()

    def load_home():
        page.controls.clear()
        page.add(home_view)
        load_files()

    page.add(login_view)

# ---------- RUN ----------
ft.run(
    main,
    view=ft.AppView.WEB_BROWSER  # <-- use enum instead of string
)