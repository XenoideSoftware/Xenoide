#!/usr/bin/env python3
"""Consumer bootstrap: the only build logic that lives outside the devkit.

`mise run devkit:sync` and `mise run setup` call this file, so they work on a
clean checkout where `.devkit/` does not exist yet. Everything after the
checkout is delegated to `.devkit/mise/scripts/xenoide_dev.py`.

Development mode: set `XENOIDE_DEVKIT_PATH=/path/to/devkit` to link `.devkit`
to a sibling checkout instead of cloning the remote.
"""

import os
import shutil
import subprocess
import sys
from pathlib import Path

DEFAULT_URL = "https://github.com/XenoideSoftware/devkit.git"
DEFAULT_REF = "v0.1.1"


def run(cmd, cwd=None):
    print("+ " + " ".join(cmd), flush=True)
    result = subprocess.run(cmd, cwd=cwd)
    if result.returncode != 0:
        raise SystemExit(result.returncode)


def git(args, cwd=None):
    result = subprocess.run(["git", *args], cwd=cwd, capture_output=True, text=True)
    if result.returncode != 0:
        raise SystemExit(f"git {' '.join(args)} failed:\n{result.stderr}")
    return result.stdout.strip()


def repo_root():
    return Path(__file__).resolve().parent.parent


def sync(root):
    ref = os.environ.get("DEVKIT_REF", DEFAULT_REF)
    url = os.environ.get("DEVKIT_URL", DEFAULT_URL)
    local = os.environ.get("XENOIDE_DEVKIT_PATH")
    devkit = root / ".devkit"

    if local:
        local_path = Path(local).expanduser().resolve()
        if not local_path.is_dir():
            raise SystemExit(f"XENOIDE_DEVKIT_PATH does not exist: {local_path}")
        if devkit.is_symlink():
            if devkit.resolve() == local_path:
                print(f"==> .devkit already links to {local_path}")
                return
            devkit.unlink()
        elif devkit.exists():
            shutil.rmtree(devkit)
        devkit.symlink_to(local_path, target_is_directory=True)
        print(f"==> Linked .devkit -> {local_path} (development mode)")
        return

    if devkit.is_symlink():
        # `.devkit` must be a real checkout, never a symlink: checking out
        # through a link would modify an unrelated sibling tree. Remove it
        # (dangling, foreign, or valid dev link) before cloning.
        devkit.unlink()

    if (devkit / ".git").is_dir():
        print(f"==> Updating .devkit to {ref}")
        subprocess.run(["git", "fetch", "--tags", "--force", url, ref],
                       cwd=devkit, check=False)
    else:
        if devkit.exists():
            shutil.rmtree(devkit)
        print(f"==> Cloning {url} -> .devkit")
        run(["git", "clone", "--quiet", url, str(devkit)])
    run(["git", "-c", "advice.detachedHead=false", "checkout", "--force", ref], cwd=devkit)


def main(argv):
    mode = argv[1] if len(argv) > 1 else "sync"
    root = repo_root()
    if mode == "sync":
        sync(root)
        return 0
    if mode != "setup":
        raise SystemExit(f"usage: {argv[0]} [sync|setup]")

    sync(root)
    script = root / ".devkit" / "mise" / "scripts" / "xenoide_dev.py"
    if not script.is_file():
        raise SystemExit(f"{script} is missing; the devkit checkout is incomplete")
    run([sys.executable, str(script), "setup"])
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv))
