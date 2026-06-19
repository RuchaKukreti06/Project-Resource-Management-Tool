import os
import sys
import re

def remove_comments(text):
    # This regex handles C/C++ comments while ignoring strings and characters.
    # It matches:
    # 1) Strings: "([^"\\]|\\.)*"
    # 2) Characters: '([^'\\]|\\.)*'
    # 3) Block comments: /\*[\s\S]*?\*/
    # 4) Line comments: //.*
    
    # We will use a function to replace matched items. If it's a string/char, we keep it.
    # If it's a comment, we replace it with a single space (or nothing if line comment).
    
    pattern = re.compile(
        r'("([^"\\]|\\.)*")|'    # Group 1: String literal
        r"('([^'\\]|\\.)*')|"    # Group 3: Character literal
        r'(/\*[\s\S]*?\*/)|'     # Group 5: Block comment
        r'(//.*)'                # Group 6: Line comment
    )

    def replacer(match):
        # If the match is a string or char, return it unchanged.
        if match.group(1) is not None:
            return match.group(1)
        if match.group(3) is not None:
            return match.group(3)
        
        # It's a comment. Return an empty string.
        # But if it's a block comment that spans multiple lines, we might want to keep the newlines
        # so line numbers don't change, but for a full strip, empty string is fine.
        return ""

    # Replace comments with empty strings
    cleaned = pattern.sub(replacer, text)
    
    # Optional: Clean up trailing whitespace and empty lines that were left behind
    lines = []
    for line in cleaned.split('\n'):
        stripped = line.rstrip()
        if stripped or not line.strip() == "": # Keep lines that are naturally empty, but don't add new ones from comments
            pass
        lines.append(stripped)
        
    return "\n".join(lines)

def process_directory(directory):
    extensions = {'.cpp', '.h', '.hpp', '.c', '.cc'}
    for root, _, files in os.walk(directory):
        if 'vcpkg' in root or 'build' in root or 'scratch' in root or '.git' in root:
            continue
        for file in files:
            if any(file.endswith(ext) for ext in extensions):
                path = os.path.join(root, file)
                try:
                    with open(path, 'r', encoding='utf-8') as f:
                        content = f.read()
                    
                    new_content = remove_comments(content)
                    
                    # Remove completely empty lines that might have been created
                    new_content = re.sub(r'\n\s*\n', '\n', new_content)
                    
                    with open(path, 'w', encoding='utf-8') as f:
                        f.write(new_content)
                    print(f"Cleaned {path}")
                except Exception as e:
                    print(f"Error processing {path}: {e}")

if __name__ == "__main__":
    target_dir = sys.argv[1] if len(sys.argv) > 1 else "."
    process_directory(target_dir)
