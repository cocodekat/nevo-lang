"""

Run with:
    pip install fastapi uvicorn
    uvicorn server:app --host 0.0.0.0 --port 8000
"""

import mimetypes
import re
from pathlib import Path
from typing import Optional

from fastapi import FastAPI, HTTPException, Request
from fastapi.middleware.cors import CORSMiddleware
from fastapi.responses import HTMLResponse, StreamingResponse

# ---------------------------------------------------------------------------
# Setup
# ---------------------------------------------------------------------------

BASE_DIR = Path(__file__).resolve().parent
FILES_DIR = BASE_DIR / "files"
FILES_DIR.mkdir(exist_ok=True)

app = FastAPI(title="File Share")

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_methods=["*"],
    allow_headers=["*"],
)

# A few extra mappings mimetypes sometimes misses.
mimetypes.add_type("video/mp4", ".mp4")
mimetypes.add_type("video/quicktime", ".mov")
mimetypes.add_type("video/webm", ".webm")
mimetypes.add_type("video/x-matroska", ".mkv")
mimetypes.add_type("text/markdown", ".md")

CATEGORY_BY_EXT = {
    "video": {"mp4", "mov", "m4v", "webm", "mkv", "avi"},
    "image": {"png", "jpg", "jpeg", "gif", "webp", "heic", "bmp"},
    "html": {"html", "htm"},
    "text": {"txt", "md", "log", "csv", "json", "py", "swift", "js", "css", "yaml", "yml", "xml"},
    "audio": {"mp3", "wav", "m4a", "flac", "aac"},
}


def categorize(filename: str) -> str:
    ext = filename.rsplit(".", 1)[-1].lower() if "." in filename else ""
    for category, exts in CATEGORY_BY_EXT.items():
        if ext in exts:
            return category
    return "other"


def human_size(num: int) -> str:
    size = float(num)
    for unit in ["B", "KB", "MB", "GB", "TB"]:
        if size < 1024:
            return f"{size:.0f} {unit}" if unit == "B" else f"{size:.1f} {unit}"
        size /= 1024
    return f"{size:.1f} PB"


def safe_path(filename: str) -> Path:
    """Resolve a filename against FILES_DIR, rejecting any path traversal."""
    candidate = (FILES_DIR / filename).resolve()
    if FILES_DIR.resolve() not in candidate.parents and candidate != FILES_DIR.resolve():
        raise HTTPException(status_code=400, detail="Invalid filename")
    if not candidate.is_file():
        raise HTTPException(status_code=404, detail="File not found")
    return candidate


# ---------------------------------------------------------------------------
# API
# ---------------------------------------------------------------------------

@app.get("/api/files")
def list_files():
    items = []
    for p in sorted(FILES_DIR.iterdir(), key=lambda p: p.name.lower()):
        if p.is_file():
            size = p.stat().st_size
            items.append({
                "name": p.name,
                "size": size,
                "size_human": human_size(size),
                "type": categorize(p.name),
                "ext": p.suffix.lstrip(".").lower(),
            })
    return items


RANGE_RE = re.compile(r"bytes=(\d*)-(\d*)")
CHUNK_SIZE = 1024 * 1024  # 1 MB


@app.get("/api/stream/{filename}")
def stream_file(filename: str, request: Request):
    path = safe_path(filename)
    file_size = path.stat().st_size
    content_type = mimetypes.guess_type(path.name)[0] or "application/octet-stream"

    range_header = request.headers.get("range")
    if range_header:
        match = RANGE_RE.match(range_header)
        if match:
            start = int(match.group(1)) if match.group(1) else 0
            end = int(match.group(2)) if match.group(2) else file_size - 1
            end = min(end, file_size - 1)
            if start > end:
                raise HTTPException(status_code=416, detail="Invalid range")
            length = end - start + 1

            def iter_range():
                with open(path, "rb") as f:
                    f.seek(start)
                    remaining = length
                    while remaining > 0:
                        chunk = f.read(min(CHUNK_SIZE, remaining))
                        if not chunk:
                            break
                        remaining -= len(chunk)
                        yield chunk

            headers = {
                "Content-Range": f"bytes {start}-{end}/{file_size}",
                "Accept-Ranges": "bytes",
                "Content-Length": str(length),
            }
            return StreamingResponse(iter_range(), status_code=206, headers=headers, media_type=content_type)

    def iter_full():
        with open(path, "rb") as f:
            while True:
                chunk = f.read(CHUNK_SIZE)
                if not chunk:
                    break
                yield chunk

    headers = {"Accept-Ranges": "bytes", "Content-Length": str(file_size)}
    return StreamingResponse(iter_full(), headers=headers, media_type=content_type)


# ---------------------------------------------------------------------------
# Browser UI
# ---------------------------------------------------------------------------

INDEX_HTML = """<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>File Share</title>
<link rel="preconnect" href="https://fonts.googleapis.com">
<link href="https://fonts.googleapis.com/css2?family=Bebas+Neue&family=Inter:wght@400;500;600;700&family=JetBrains+Mono:wght@400;500&display=swap" rel="stylesheet">
<style>
  :root {
    --bg: #0A0A0D;
    --bg-elevated: #141418;
    --card: #1C1C22;
    --text-primary: #F5F5F0;
    --text-secondary: #8B8B94;
    --video: #E4572E;
    --image: #A66CFF;
    --html: #FF9F1C;
    --text: #2EC4B6;
    --audio: #E0C341;
    --other: #5C5C66;
  }
  * { box-sizing: border-box; }
  body {
    margin: 0;
    background: var(--bg);
    color: var(--text-primary);
    font-family: 'Inter', sans-serif;
    -webkit-font-smoothing: antialiased;
  }
  header.hero {
    padding: 64px 5vw 40px;
    background: linear-gradient(180deg, #17171C 0%, var(--bg) 100%);
    border-bottom: 1px solid #1F1F26;
  }
  .wordmark {
    font-family: 'Bebas Neue', sans-serif;
    font-size: clamp(48px, 8vw, 84px);
    letter-spacing: 3px;
    line-height: 1;
    margin: 0;
    background: linear-gradient(90deg, #fff, #9a9aa4);
    -webkit-background-clip: text;
    background-clip: text;
    color: transparent;
  }
  .tagline {
    color: var(--text-secondary);
    font-size: 15px;
    margin-top: 10px;
    letter-spacing: 0.3px;
  }
  main { padding: 8px 5vw 80px; }
  .shelf { margin-top: 44px; }
  .shelf-title {
    font-family: 'Bebas Neue', sans-serif;
    font-size: 22px;
    letter-spacing: 1.5px;
    color: var(--text-secondary);
    margin: 0 0 14px 2px;
  }
  .row {
    display: flex;
    gap: 16px;
    overflow-x: auto;
    padding-bottom: 12px;
    scroll-snap-type: x proximity;
  }
  .row::-webkit-scrollbar { height: 6px; }
  .row::-webkit-scrollbar-thumb { background: #2A2A32; border-radius: 3px; }
  .card {
    flex: 0 0 auto;
    width: 168px;
    scroll-snap-align: start;
    cursor: pointer;
    transition: transform 0.18s ease;
  }
  .card:hover { transform: translateY(-6px); }
  .poster {
    width: 168px;
    height: 236px;
    border-radius: 10px;
    display: flex;
    align-items: center;
    justify-content: center;
    position: relative;
    overflow: hidden;
    box-shadow: 0 10px 24px rgba(0,0,0,0.45);
  }
  .poster .ext {
    font-family: 'Bebas Neue', sans-serif;
    font-size: 40px;
    letter-spacing: 2px;
    color: rgba(255,255,255,0.92);
    z-index: 1;
  }
  .poster::after {
    content: "";
    position: absolute;
    inset: 0;
    background: linear-gradient(180deg, rgba(0,0,0,0) 40%, rgba(0,0,0,0.55) 100%);
  }
  .card-title {
    margin-top: 9px;
    font-size: 13px;
    font-weight: 500;
    color: var(--text-primary);
    white-space: nowrap;
    overflow: hidden;
    text-overflow: ellipsis;
  }
  .card-meta {
    font-family: 'JetBrains Mono', monospace;
    font-size: 11px;
    color: var(--text-secondary);
    margin-top: 2px;
  }
  .empty {
    color: var(--text-secondary);
    padding: 60px 0;
    text-align: center;
    font-size: 15px;
  }

  /* Player modal */
  .modal {
    position: fixed;
    inset: 0;
    background: rgba(5,5,7,0.92);
    display: none;
    align-items: center;
    justify-content: center;
    z-index: 100;
    padding: 4vh 4vw;
  }
  .modal.open { display: flex; }
  .modal-inner {
    width: 100%;
    height: 100%;
    max-width: 1100px;
    background: var(--bg-elevated);
    border-radius: 12px;
    overflow: hidden;
    position: relative;
    display: flex;
    flex-direction: column;
  }
  .modal-header {
    display: flex;
    align-items: center;
    justify-content: space-between;
    padding: 14px 18px;
    border-bottom: 1px solid #232329;
  }
  .modal-header span { font-size: 14px; color: var(--text-secondary); font-family: 'JetBrains Mono', monospace; }
  .close-btn {
    background: none;
    border: none;
    color: var(--text-primary);
    font-size: 22px;
    cursor: pointer;
    line-height: 1;
    padding: 4px 8px;
  }
  .modal-body { flex: 1; overflow: auto; background: #000; display: flex; align-items: center; justify-content: center; }
  .modal-body video, .modal-body img { max-width: 100%; max-height: 100%; }
  .modal-body iframe { width: 100%; height: 100%; border: none; background: #fff; }
  .modal-body pre {
    width: 100%;
    height: 100%;
    margin: 0;
    padding: 24px;
    color: #D8D8DE;
    font-family: 'JetBrains Mono', monospace;
    font-size: 13px;
    line-height: 1.6;
    white-space: pre-wrap;
    word-break: break-word;
    overflow: auto;
    background: var(--bg-elevated);
  }
  .modal-body audio { width: 80%; }
  .fallback { color: var(--text-secondary); text-align: center; padding: 40px; }
  .fallback a { color: var(--video); }
</style>
</head>
<body>

<header class="hero">
  <h1 class="wordmark">File Share</h1>
  <div class="tagline">Everything in your files/ folder, organized like it deserves to be.</div>
</header>

<main id="main"></main>

<div class="modal" id="modal">
  <div class="modal-inner">
    <div class="modal-header">
      <span id="modal-filename"></span>
      <button class="close-btn" onclick="closeModal()">&times;</button>
    </div>
    <div class="modal-body" id="modal-body"></div>
  </div>
</div>

<script>
const CATEGORY_ORDER = [
  ["video", "Videos"],
  ["html", "Web Pages"],
  ["text", "Documents"],
  ["image", "Images"],
  ["audio", "Audio"],
  ["other", "Other"],
];

async function loadFiles() {
  const main = document.getElementById('main');
  let files = [];
  try {
    const res = await fetch('/api/files');
    files = await res.json();
  } catch (e) {
    main.innerHTML = '<div class="empty">Could not reach the server.</div>';
    return;
  }

  if (files.length === 0) {
    main.innerHTML = '<div class="empty">No files yet — drop something into the files/ folder.</div>';
    return;
  }

  const byCategory = {};
  for (const f of files) {
    (byCategory[f.type] ||= []).push(f);
  }

  main.innerHTML = '';
  for (const [key, label] of CATEGORY_ORDER) {
    const items = byCategory[key];
    if (!items || items.length === 0) continue;

    const shelf = document.createElement('div');
    shelf.className = 'shelf';

    const title = document.createElement('div');
    title.className = 'shelf-title';
    title.textContent = `${label} (${items.length})`;
    shelf.appendChild(title);

    const row = document.createElement('div');
    row.className = 'row';

    for (const f of items) {
      row.appendChild(makeCard(f));
    }

    shelf.appendChild(row);
    main.appendChild(shelf);
  }
}

function makeCard(f) {
  const card = document.createElement('div');
  card.className = 'card';
  card.onclick = () => openModal(f);

  const poster = document.createElement('div');
  poster.className = 'poster';
  poster.style.background = `linear-gradient(160deg, var(--${f.type}, var(--other)) 0%, #0000 140%), #17171C`;
  poster.innerHTML = `<span class="ext">${(f.ext || '?').toUpperCase()}</span>`;

  const title = document.createElement('div');
  title.className = 'card-title';
  title.textContent = f.name;

  const meta = document.createElement('div');
  meta.className = 'card-meta';
  meta.textContent = f.size_human;

  card.appendChild(poster);
  card.appendChild(title);
  card.appendChild(meta);
  return card;
}

async function openModal(f) {
  const modal = document.getElementById('modal');
  const body = document.getElementById('modal-body');
  document.getElementById('modal-filename').textContent = f.name;
  body.innerHTML = '';

  const url = `/api/stream/${encodeURIComponent(f.name)}`;

  if (f.type === 'video') {
    body.innerHTML = `<video src="${url}" controls autoplay></video>`;
  } else if (f.type === 'audio') {
    body.innerHTML = `<audio src="${url}" controls autoplay></audio>`;
  } else if (f.type === 'image') {
    body.innerHTML = `<img src="${url}">`;
  } else if (f.type === 'html') {
    body.innerHTML = `<iframe src="${url}" sandbox="allow-scripts allow-same-origin"></iframe>`;
  } else if (f.type === 'text') {
    try {
      const res = await fetch(url);
      const text = await res.text();
      const pre = document.createElement('pre');
      pre.textContent = text;
      body.appendChild(pre);
    } catch (e) {
      body.innerHTML = `<div class="fallback">Couldn't load this file.</div>`;
    }
  } else {
    body.innerHTML = `<div class="fallback">No preview available.<br><br><a href="${url}" target="_blank">Open / download</a></div>`;
  }

  modal.classList.add('open');
}

function closeModal() {
  document.getElementById('modal').classList.remove('open');
  document.getElementById('modal-body').innerHTML = '';
}

document.getElementById('modal').addEventListener('click', (e) => {
  if (e.target.id === 'modal') closeModal();
});
document.addEventListener('keydown', (e) => {
  if (e.key === 'Escape') closeModal();
});

loadFiles();
</script>
</body>
</html>
"""


@app.get("/", response_class=HTMLResponse)
def index():
    return INDEX_HTML