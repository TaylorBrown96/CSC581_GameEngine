import os
from PIL import Image

# Input and output directories
target_folder = "media"

# Create output folder if it doesn't exist
os.makedirs(target_folder, exist_ok=True)

# Loop through all files in the input folder
for filename in os.listdir(target_folder):
    file_path = os.path.join(target_folder, filename)

    # Skip non-files (like directories)
    if not os.path.isfile(file_path):
        continue

    try:
        # Open image
        with Image.open(file_path) as img:
            # Get base name without extension
            base_name = os.path.splitext(filename)[0]
            # Set new filename with .bmp extension
            bmp_path = os.path.join(target_folder, f"{base_name}.bmp")

            # Save as BMP
            img.save(bmp_path, "BMP")
            print(f"Converted {filename} -> {bmp_path}")
    except Exception as e:
        print(f"Skipping {filename}: {e}")
