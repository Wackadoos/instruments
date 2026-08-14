Import("env")
import shutil, os, glob


def patch_neogps(*args, **kwargs):
    overrides_dir = os.path.join(env["PROJECT_DIR"], "lib", "cfg", "NeoGPS")
    candidates = glob.glob(
        os.path.join(env["PROJECT_LIBDEPS_DIR"], env["PIOENV"], "NeoGPS", "src")
    )
    if not candidates:
        print("NeoGPS not found yet (first run before download) - skipping patch")
        return
    dest = candidates[0]
    for fname in os.listdir(overrides_dir):
        shutil.copyfile(os.path.join(overrides_dir, fname), os.path.join(dest, fname))
        print(f"Patched {fname} -> {dest}")


patch_neogps()
