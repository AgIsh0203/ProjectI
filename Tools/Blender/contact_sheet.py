# Tiles PNGs (same size) into one sheet, row by row.
# blender --background --factory-startup --python contact_sheet.py -- <out.png> <cols> <img> [img ...]
import sys

import bpy
import numpy as np

args = sys.argv[sys.argv.index("--") + 1:]
out, cols, paths = args[0], int(args[1]), args[2:]
imgs = [bpy.data.images.load(p) for p in paths]
w, h = imgs[0].size
rows = (len(imgs) + cols - 1) // cols
sheet = np.zeros((rows * h, cols * w, 4), dtype=np.float32)
for i, im in enumerate(imgs):
    px = np.array(im.pixels[:], dtype=np.float32).reshape(h, w, 4)  # bottom row first
    r, c = divmod(i, cols)
    y0 = (rows - 1 - r) * h
    sheet[y0:y0 + h, c * w:(c + 1) * w] = px
res = bpy.data.images.new("Sheet", cols * w, rows * h, alpha=True)
res.pixels = sheet.ravel()
res.filepath_raw = out
res.file_format = "PNG"
res.save()
print("SHEET_DONE")
