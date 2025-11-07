#!/usr/bin/env python3
import sys
import os
import json
import argparse
import hashlib
import shutil
from pathlib import Path
from zipfile import ZipFile, BadZipFile
from urllib.parse import urlparse

try:
    import requests
except ImportError:
    requests = None
    import urllib.request

INSTALL_ROOT = Path.home() / "Downloads"
GITHUB_INDEX_URL = "https://github.com/cocodekat/Nevo/raw/main/index.json"

def load_remote_index():
    """Load index.json from GitHub and return the 'packages' dictionary."""
    if requests:
        r = requests.get(GITHUB_INDEX_URL, timeout=30)
        r.raise_for_status()
        data = r.json()
    else:
        with urllib.request.urlopen(GITHUB_INDEX_URL) as r:
            data = json.load(r)

    return data.get("packages", {})

def _choose_latest_version(versions):
    """Sort versions and pick the latest."""
    def key(v):
        return tuple(int(x) if x.isdigit() else x for x in v.split("."))
    return sorted(versions, key=key)[-1]

def find_package_entry(index, name, version=None):
    """Find a package entry and version from the index."""
    if name not in index:
        return None, None
    pkg_versions = index[name]
    if version:
        entry = pkg_versions.get(version)
        return (version, entry) if entry else (None, None)
    # Pick latest version if version not specified
    latest = _choose_latest_version(list(pkg_versions.keys()))
    return latest, pkg_versions[latest]

def verify_sha256(path, expected):
    if not expected:
        return True
    h = hashlib.sha256()
    with open(path, "rb") as f:
        for chunk in iter(lambda: f.read(8192), b""):
            h.update(chunk)
    return h.hexdigest().lower() == expected.lower()

def print_progress_bar(pct, width=40):
    filled = int(width * pct / 100)
    bar = "#" * filled + "-" * (width - filled)
    sys.stdout.write(f"\r[{bar}] {pct:3d}%")
    sys.stdout.flush()

def get_downloads_folder():
    return Path.home() / "Downloads"

def download_to_downloads(url, expected_sha256=None):
    # Convert GitHub blob URL to raw URL if needed
    if url and "github.com" in url and "/blob/" in url:
        url = url.replace("/blob/", "/raw/")

    downloads_dir = get_downloads_folder()
    downloads_dir.mkdir(parents=True, exist_ok=True)
    fname = os.path.basename(urlparse(url).path) or "package.zip"
    file_path = downloads_dir / fname

    if requests:
        with requests.get(url, stream=True, timeout=60) as r:
            r.raise_for_status()
            total = r.headers.get("content-length")
            total = int(total) if total and total.isdigit() else None
            downloaded = 0
            last_pct = 0
            with open(file_path, "wb") as fh:
                for chunk in r.iter_content(chunk_size=8192):
                    if not chunk:
                        continue
                    fh.write(chunk)
                    if total:
                        downloaded += len(chunk)
                        pct = downloaded * 100 // total
                        if pct != last_pct:
                            print_progress_bar(pct)
                            last_pct = pct
            if total:
                print()
    else:
        urllib.request.urlretrieve(url, file_path)

    if not verify_sha256(file_path, expected_sha256):
        os.remove(file_path)
        sys.exit("SHA256 mismatch — aborting.")
    return str(file_path)

def safe_extract_zip(zip_path, dest_dir):
    try:
        with ZipFile(zip_path, 'r') as z:
            for member in z.namelist():
                member_path = Path(dest_dir) / member
                if not Path(dest_dir).resolve() in member_path.resolve().parents and Path(dest_dir).resolve() != member_path.resolve():
                    raise BadZipFile("Zip contains unsafe paths")
            z.extractall(dest_dir)
    except BadZipFile as e:
        sys.exit(f"Failed to extract zip: {e}")

def install(name, version=None):
    index = load_remote_index()
    v, entry = find_package_entry(index, name, version)
    if not entry:
        sys.exit(f"Package '{name}'{' version '+version if version else ''} not found in index")

    url = entry.get("download_url") or entry.get("url") or entry.get("link")
    expected_sha = entry.get("sha256") or entry.get("sha")

    print(f"Installing {name} version {v}")
    tmpfile = download_to_downloads(url, expected_sha256=expected_sha)

    pkg_dir = INSTALL_ROOT / name / v
    if pkg_dir.exists():
        shutil.rmtree(pkg_dir)
    pkg_dir.mkdir(parents=True, exist_ok=True)

    if tmpfile.lower().endswith(".zip") or url.lower().endswith(".zip"):
        safe_extract_zip(tmpfile, str(pkg_dir))
    else:
        dest = pkg_dir / os.path.basename(tmpfile)
        shutil.move(tmpfile, dest)

    # Run post-install scripts if any
    after_cmd = None
    if sys.platform.startswith("win"):
        after_cmd = entry.get("after_windows")
    elif sys.platform == "darwin":
        after_cmd = entry.get("after_macos")

    if after_cmd:
        os.chdir(pkg_dir)
        try:
            os.system(after_cmd)
        except Exception as e:
            print(f"⚠️  Failed to run post-install command: {e}")

    print("Done!")

def list_packages():
    index = load_remote_index()
    for pkg, versions in index.items():
        latest = _choose_latest_version(list(versions.keys()))
        url = versions[latest].get("download_url") or versions[latest].get("url") or versions[latest].get("link", "")
        print(f"{pkg} {latest} -> {url}")

def parse_pkg_spec(spec):
    """Supports 'name@version' format."""
    if "@" in spec:
        name, ver = spec.split("@", 1)
        return name, ver
    return spec, None

def main(argv):
    p = argparse.ArgumentParser(prog="package_manager", description="Simple GitHub-based package installer")
    p.add_argument("-install", "--install", metavar="PKG", help="Install package (use name or name@version)")
    p.add_argument("-v", "--version", metavar="VER", help="Specify version to install")
    p.add_argument("-list", "--list", action="store_true", help="List packages in index")
    args = p.parse_args(argv)

    if args.list:
        list_packages()
        return

    if args.install:
        name, ver_from_spec = parse_pkg_spec(args.install)
        ver = args.version or ver_from_spec
        install(name, ver)
        return

    p.print_help()

if __name__ == "__main__":
    main(sys.argv[1:])
