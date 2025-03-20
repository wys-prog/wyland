import argparse
import os
import tempfile
import markdown
import subprocess

# Define the allowed file for safety
ALLOWED_FILE = "/Users/wys/Documents/wyweb/announcement.html"
ANNOUNCEMENT_START = '<div class="announcement" data-name="{name}">'
ANNOUNCEMENT_END = "</div>"

def validate_file(file_path):
    """Ensures only the allowed file is modified."""
    if os.path.abspath(file_path) != os.path.abspath(ALLOWED_FILE):
        raise ValueError("❌ Error: Modifications are restricted to the allowed file.")

def open_editor(initial_content=""):
    """Opens the system's default text editor for input."""
    with tempfile.NamedTemporaryFile(suffix=".tmp", mode="w+", delete=False) as tmpfile:
        tmpfile.write(initial_content)
        tmpfile.flush()
        subprocess.run([os.getenv("EDITOR", "nano"), tmpfile.name])
        tmpfile.seek(0)
        content = tmpfile.read()
    os.unlink(tmpfile.name)
    return content.strip()

def add_announcement(file_path, name, content, format_md=False):
    """Adds an announcement to the HTML file."""
    validate_file(file_path)
    mode = "a" if os.path.exists(file_path) else "w"
    with open(file_path, mode, encoding="utf-8") as f:
        f.write("\n" + ANNOUNCEMENT_START.format(name=name) + "\n")
        if format_md:
            content = markdown.markdown(content)
        f.write(content + "\n" + ANNOUNCEMENT_END + "\n")
    print(f"✅ Announcement '{name}' added successfully.")

def remove_announcement(file_path, name):
    """Removes an announcement by name."""
    validate_file(file_path)
    temp_file = tempfile.NamedTemporaryFile(delete=False, mode="w", encoding="utf-8")
    inside_announcement = False
    found = False

    with open(file_path, "r", encoding="utf-8") as f, temp_file:
        for line in f:
            if ANNOUNCEMENT_START.format(name=name) in line:
                inside_announcement = True
                found = True
                continue
            if inside_announcement and ANNOUNCEMENT_END in line:
                inside_announcement = False
                continue
            if not inside_announcement:
                temp_file.write(line)

    os.replace(temp_file.name, file_path)
    print("🗑️ Announcement removed." if found else "⚠️ Announcement not found.")

def clear_announcements(file_path):
    """Removes all announcements."""
    validate_file(file_path)
    temp_file = tempfile.NamedTemporaryFile(delete=False, mode="w", encoding="utf-8")
    with open(file_path, "r", encoding="utf-8") as f, temp_file:
        for line in f:
            if "<div class=\"announcement\"" in line:
                continue
            temp_file.write(line)
    os.replace(temp_file.name, file_path)
    print("🚮 All announcements have been removed.")

def list_announcements(file_path):
    """Lists all announcements by name."""
    validate_file(file_path)
    print("📋 List of announcements:")
    with open(file_path, "r", encoding="utf-8") as f:
        for line in f:
            if '<div class="announcement"' in line:
                name = line.split('data-name="')[1].split('"')[0]
                print(f" - {name}")

def main():
    parser = argparse.ArgumentParser(description="Manage HTML announcements.")
    subparsers = parser.add_subparsers(dest="command")

    # Add announcement
    parser_add = subparsers.add_parser("add", help="Add a new announcement")
    parser_add.add_argument("name", type=str, help="Unique name for the announcement")
    parser_add.add_argument("--content", type=str, help="Text content or file path")
    parser_add.add_argument("--md", action="store_true", help="Format content as Markdown")

    # Remove announcement
    parser_remove = subparsers.add_parser("remove", help="Remove an announcement by name")
    parser_remove.add_argument("name", type=str, help="Name of the announcement to remove")

    # Clear all announcements
    subparsers.add_parser("clear", help="Remove all announcements")

    # List announcements
    subparsers.add_parser("list", help="List all announcements")

    args = parser.parse_args()

    if args.command == "add":
        if args.content and os.path.exists(args.content):
            with open(args.content, "r", encoding="utf-8") as f:
                content = f.read().strip()
        elif args.content:
            content = args.content
        else:
            print("✏️ Opening editor... Save and exit when done.")
            content = open_editor()
        add_announcement(ALLOWED_FILE, args.name, content, args.md)
    elif args.command == "remove":
        remove_announcement(ALLOWED_FILE, args.name)
    elif args.command == "clear":
        clear_announcements(ALLOWED_FILE)
    elif args.command == "list":
        list_announcements(ALLOWED_FILE)
    else:
        parser.print_help()

if __name__ == "__main__":
    main()