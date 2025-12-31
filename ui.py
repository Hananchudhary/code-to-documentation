import streamlit as st
import socket
import json
from Huffman import HuffmanNode
from typing import Dict, Any, Optional

# ---------------- Configuration ----------------
SERVER_HOST = "127.0.0.1"
SERVER_PORT = 8080
BUFFER_SIZE = 10000

# ---------------- TCP Client ----------------
def send_request(payload: Dict[str, Any]) -> Optional[Dict]:
    """Send JSON request to C++ server via TCP"""
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.connect((SERVER_HOST, SERVER_PORT))
        s.send(json.dumps(payload).encode())
        data = s.recv(BUFFER_SIZE).decode()
        s.close()
        return json.loads(data) 
    except Exception as e:
        st.error(f"❌ Connection error: {e}")
        return None

# ---------------- Session State Initialization ----------------
if 'logged_in' not in st.session_state:
    st.session_state.logged_in = False
if 'username' not in st.session_state:
    st.session_state.username = ""
if 'editor' not in st.session_state:
    st.session_state.editor = ""
if 'current_file' not in st.session_state:
    st.session_state.current_file = None

# ---------------- Login Page ----------------
def login_page():
    st.title("🔐 Omni Docs - Login")
    with st.form("login_form"):
        username = st.text_input("Username", key="login_user")
        password = st.text_input("Password", type="password", key="login_pass")
        submit = st.form_submit_button("Login")
        if submit:
            if username and password:
                resp = send_request({
                    "operation": "login",
                    "parameters": {"username": username, "password": password}
                })
                if resp and resp.get("status") == "success":
                    st.session_state.logged_in = True
                    st.session_state.username = username
                    st.success("✅ Login successful!")
                    st.rerun()()
                else:
                    st.error(resp.get("error_message", "Login failed"))
            else:
                st.warning("⚠️ Enter both username and password")

# ---------------- Home / My Files ----------------
def show_my_files():
    st.header("📂 My Files")
    if st.button("🔄 Refresh Files", key="refresh_files"):
        st.rerun()()

    resp = send_request({"operation": "get_files", "parameters": {"username": st.session_state.username}})
    files = resp.get("data", {}).get("files", []) if resp and resp.get("status") == "success" else []

    if not files:
        st.info("No files found. Create your first file!")
        return

    st.success(f"Found {len(files)} file(s)")
    for i, fname in enumerate(files, 1):
        col1, col2, col3 = st.columns([4, 1, 1])
        with col1:
            st.write(f"{i}. 📄 {fname}")
        with col2:
            if st.button("Open", key=f"open_{fname}"):
                read_file(fname)
        with col3:
            if st.button("Share", key=f"share_{fname}"):
                share_file_dialog(fname)

# ---------------- Read File ----------------
def read_file(filename: str):
    resp = send_request({
        "operation": "read_file",
        "parameters": {"username": st.session_state.username, "filename": filename}
    })
    if resp and resp.get("status") == "success":
        st.session_state.current_file = filename
        st.session_state.editor = resp["data"]["result_data"]
        st.rerun()()
    else:
        st.error(resp.get("error_message", "Failed to read file"))

# ---------------- Edit File ----------------
def edit_file():
    if not st.session_state.current_file:
        st.warning("Select a file first")
        return
    with st.form("edit_file_form"):
        content = st.text_area("Edit File", st.session_state.editor, height=300, key=f"edit_{st.session_state.current_file}")
        submit = st.form_submit_button("Save")
        if submit:
            resp = send_request({
                "operation": "edit_file",
                "parameters": {"username": st.session_state.username, "filename": st.session_state.current_file, "data": content}
            })
            if resp and resp.get("status") == "success":
                st.success(f"✅ File '{st.session_state.current_file}' saved successfully!")
                st.session_state.editor = content
            else:
                st.error(resp.get("error_message", "Save failed"))

# ---------------- Generate Documentation ----------------
def generate_document():

    st.header("📄 Generate Document (Decoded)")
    if not st.session_state.current_file:
        st.warning("Select a file first")
        return
    filename = st.session_state.current_file
    
    if st.button("Generate Document"):
        if not filename:
            st.warning("⚠️ Please enter a filename")
            return

        # Send request to C++ server
        resp = send_request({
            "operation" : "generate_doc",
            "parameters" : {
                "username": st.session_state.username,
                "filename": filename
            }
        })

        if resp and resp.get("status") == "success":
            data = resp["data"]

            # Convert frequency table from list of [char_code, freq] to dict
            freq_table_list = data.get("freq_table", [])
            freq_table = {chr(char_code): freq for char_code, freq in freq_table_list}

            encoded_data = data.get("encoded_data", "")
            decoded_text = HuffmanNode.decode(encoded_data, freq_table)

            st.success("✅ Document generated and decoded!")
            st.text_area("Decoded Document:", value=decoded_text, height=300, disabled=True)
        else:
            st.error(resp.get("error_message", "Document generation failed"))


# ---------------- File Search ----------------
def file_search():
    prefix = st.text_input("Search by prefix", key="search_prefix")

    if "search_results" not in st.session_state:
        st.session_state.search_results = []

    if st.button("Search", key="search_btn"):
        resp = send_request({
            "operation": "get_file_names",
            "parameters": {
                "username": st.session_state.username,
                "prefix": prefix
            }
        })

        if resp and resp.get("status") == "success":
            st.session_state.search_results = resp.get("data", {}).get("file_names", [])
        else:
            st.session_state.search_results = []

    file_names = st.session_state.search_results

    if file_names:
        st.success(f"Found {len(file_names)} file(s)")
        for i, fname in enumerate(file_names, 1):
            col1, col2 = st.columns([4, 1])

            with col1:
                st.write(f"{i}. 📄 {fname}")

            with col2:
                if st.button("Open", key=f"search_open_{i}"):
                    read_file(fname)
    else:
        st.info("No matching files found")

# ---------------- Share File ----------------
def share_file_dialog(filename=None):
    if not filename: filename = st.session_state.current_file
    if not filename:
        st.warning("Select a file first")
        return
    with st.form(f"share_form_{filename}"):
        target = st.text_input("Share with user", key=f"share_target_{filename}")
        submit = st.form_submit_button("Share File")
        if submit and target:
            resp = send_request({
                "operation": "share_file",
                "parameters": {"owner": st.session_state.username, "filename": filename, "target_user": target}
            })
            if resp and resp.get("status") == "success":
                st.success("✅ File shared successfully!")
                st.info(f"Share key: {resp['data']['result_data']}")
            else:
                st.error(resp.get("error_message", "Share failed"))

# ---------------- Read Shared File ----------------
def read_shared_file():
    with st.form("read_shared_form"):
        key = st.text_input("Enter share key", key="shared_key")
        submit = st.form_submit_button("Read Shared File")
        if submit and key:
            resp = send_request({
                "operation": "read_shared_file",
                "parameters": {"username": st.session_state.username, "key": key}
            })
            if resp and resp.get("status") == "success":
                st.session_state.current_file = None
                st.session_state.editor = resp["data"]["result_data"]
                st.text_area("File Content", st.session_state.editor, height=300, disabled=True)
            else:
                st.error(resp.get("error_message", "Failed to read shared file"))

# ---------------- Create File ----------------
def create_file():
    with st.form("create_file_form"):
        filename = st.text_input("Filename", key="create_filename")
        content = st.text_area("Initial content", height=200, key="create_content")
        submit = st.form_submit_button("Create File")
        if submit and filename:
            resp = send_request({
                "operation": "create_file",
                "parameters": {"username": st.session_state.username, "filename": filename, "data": content}
            })
            if resp and resp.get("status") == "success":
                st.success(f"✅ File '{filename}' created!")
            else:
                st.error(resp.get("error_message", "File creation failed"))

# ---------------- Main App ----------------
def main():
    st.set_page_config(page_title="Omni Docs", layout="wide")
    st.title("📁 Omni Docs")
    if not st.session_state.logged_in:
        login_page()
        return

    st.sidebar.header(f"👤 {st.session_state.username}")
    if st.sidebar.button("Logout", key="logout"):
        st.session_state.logged_in = False
        st.session_state.username = ""
        st.rerun()()

    menu = st.sidebar.radio(
        "Menu",
        ["My Files", "Create File", "Edit File", "Generate Document", "Search Files", "Share File", "Read Shared File"]
    )

    if menu == "My Files":
        show_my_files()
    elif menu == "Create File":
        create_file()
    elif menu == "Edit File":
        edit_file()
    elif menu == "Generate Document":
        generate_document()
    elif menu == "Search Files":
        file_search()
    elif menu == "Share File":
        share_file_dialog()
    elif menu == "Read Shared File":
        read_shared_file()

if __name__ == "__main__":
    main()
