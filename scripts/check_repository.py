from __future__ import annotations

import json
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]


descriptor_paths = [ROOT / "ShibMVMain.uproject", *ROOT.glob("Plugins/**/*.uplugin")]
if len(descriptor_paths) < 2:
    raise SystemExit("Expected the Unreal project and plugin descriptors")

for path in descriptor_paths:
    with path.open(encoding="utf-8-sig") as handle:
        json.load(handle)

sensitive_settings = {
    "Config/DedicatedServerEngine.ini": {
        "DedicatedServerClientSecret",
        "DedicatedServerPrivateKey",
    },
    "Config/DefaultEditor.ini": {
        "DedicatedServerClientSecret",
        "DedicatedServerPrivateKey",
    },
    "Config/DefaultEngine.ini": {
        "SecurityToken",
        "ClientSecret",
        "PlayerDataEncryptionKey",
    },
    "Config/DefaultGame.ini": {"DedicatedServerApiKey"},
}

for relative_path, expected_keys in sensitive_settings.items():
    values: dict[str, str] = {}
    for raw_line in (ROOT / relative_path).read_text(encoding="utf-8-sig").splitlines():
        if "=" not in raw_line:
            continue
        key, value = raw_line.split("=", 1)
        if key.strip() in expected_keys:
            values[key.strip()] = value.strip()
    missing = expected_keys.difference(values)
    configured = sorted(key for key, value in values.items() if value)
    if missing or configured:
        raise SystemExit(
            f"Unsafe tracked configuration in {relative_path}: "
            f"missing={sorted(missing)}, configured={configured}"
        )

print(f"Validated {len(descriptor_paths)} Unreal descriptors and tracked configuration.")
