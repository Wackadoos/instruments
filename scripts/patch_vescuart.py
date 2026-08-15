Import("env")
import os, glob, shutil


def patch_vescuart(*args, **kwargs):
    overrides_dir = os.path.join(env["PROJECT_DIR"], "lib", "cfg", "VescUart")
    candidates = glob.glob(
        os.path.join(env["PROJECT_LIBDEPS_DIR"], env["PIOENV"], "VescUart", "src")
    )
    if not candidates:
        print("VescUart not found yet (first run before download) - skipping patch")
        return
    dest = candidates[0]
    for fname in os.listdir(overrides_dir):
        shutil.copyfile(os.path.join(overrides_dir, fname), os.path.join(dest, fname))
        print(f"Patched {fname} -> {dest}")


patch_vescuart()
