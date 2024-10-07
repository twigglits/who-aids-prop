#!/bin/bash

# Loop through each line in notes.txt and remove matching lines from test_config1.txt
while IFS= read -r line; do
    # Print the current line being processed from notes.txt for debugging
    echo "Processing: $line"

    # Escape special characters in the line
    escaped_line=$(echo "$line" | sed 's/[\/&]/\\&/g')

    # Remove lines from test_config1.txt that match the pattern, ignoring lines starting with '#'
    grep -v -E "^#.*|.*$escaped_line.*" test_config1.txt > temp && mv temp test_config1.txt
done < notes.txt

# We need to execute script.sh to remove all occurences of the variable.