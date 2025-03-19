# /Users/wys/Documents/wyweb/announcement.html
import argparse
import os
import tempfile
import markdown

ANNOUNCEMENT_START = '<div class="announcement" data-name="{name}">'
ANNOUNCEMENT_END = '</div>'

def add_announcement(file_path, name, content, format_md=False):
    """Adds an announcement to the HTML file with a unique name."""
    mode = "a" if os.path.exists(file_path) else "w"
    with open(file_path, mode, encoding="utf-8") as f:
        f.write("\n")
        f.write(ANNOUNCEMENT_START.format(name=name) + "\n")
        if format_md:
            content = markdown.markdown(content)
        f.write(content + "\n")
        f.write(ANNOUNCEMENT_END + "\n")
    print(f"✅ Announcement '{name}' added successfully.")

def remove_announcement(file_path, name):
    """Removes an announcement by searching for its name."""
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

    if found:
        print(f"🗑️ Announcement '{name}' removed.")
    else:
        print(f"⚠️ Announcement '{name}' not found.")

def clear_announcements(file_path):
    """Removes all announcements."""
    temp_file = tempfile.NamedTemporaryFile(delete=False, mode="w", encoding="utf-8")

    with open(file_path, "r", encoding="utf-8") as f, temp_file:
        inside_announcement = False
        for line in f:
            if '<div class="announcement"' in line:
                inside_announcement = True
            if inside_announcement and ANNOUNCEMENT_END in line:
                inside_announcement = False
                continue
            if not inside_announcement:
                temp_file.write(line)

    os.replace(temp_file.name, file_path)
    print("🚮 All announcements have been removed.")

def interactive_mode(file_path):
    """Interactive mode for managing announcements."""
    print("🔧 Interactive mode activated (type 'exit' to quit)")
    while True:
        action = input("Command (add/remove/clear/list/exit) ? ").strip().lower()
        if action == "exit":
            break
        elif action == "add":
            name = input("Announcement name: ").strip()
            print("Enter announcement content (end with an empty line):")
            content_lines = []
            while True:
                line = input()
                if line == "":
                    break
                content_lines.append(line)
            content = "\n".join(content_lines)
            format_md = input("Use Markdown? (y/n) ").strip().lower() == "y"
            add_announcement(file_path, name, content, format_md)
        elif action == "remove":
            name = input("Name of the announcement to remove: ").strip()
            remove_announcement(file_path, name)
        elif action == "clear":
            confirm = input("⚠️ Remove all announcements? (y/n) ").strip().lower()
            if confirm == "y":
                clear_announcements(file_path)
        elif action == "list":
            print("📋 List of announcements:")
            with open(file_path, "r", encoding="utf-8") as f:
                for line in f:
                    if '<div class="announcement"' in line:
                        name = line.split('data-name="')[1].split('"')[0]
                        print(f" - {name}")
        else:
            print("❌ Unknown command.")

def main():
    parser = argparse.ArgumentParser(description="Manage HTML announcements in a file.")
    parser.add_argument("file", type=str, help="Target HTML file (e.g., output.txt)")
    parser.add_argument("--add", nargs=2, metavar=("NAME", "CONTENT"), help="Add an announcement")
    parser.add_argument("--remove", metavar="NAME", help="Remove an announcement by name")
    parser.add_argument("--clear", action="store_true", help="Remove all announcements")
    parser.add_argument("--md", action="store_true", help="Use Markdown for formatting content")
    
    args = parser.parse_args()

    if args.add:
        name, content = args.add
        add_announcement(args.file, name, content, args.md)
    elif args.remove:
        remove_announcement(args.file, args.remove)
    elif args.clear:
        clear_announcements(args.file)
    else:
        interactive_mode(args.file)

if __name__ == "__main__":
    main()