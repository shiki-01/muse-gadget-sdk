"""Mugi: components/mugi_wifi_enterprise の単体テスト（ホスト、IDF 不要）。"""

from __future__ import annotations

import shlex
import shutil
import subprocess
import tempfile
import unittest
import os
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
COMP = ROOT / "components" / "mugi_wifi_enterprise"


def run_harness(defines: list[str]) -> subprocess.CompletedProcess:
    cc = shlex.split(os.environ.get("CC", "cc"))
    if not cc or shutil.which(cc[0]) is None:
        raise unittest.SkipTest("C compiler not available")
    with tempfile.TemporaryDirectory() as tmpdir:
        binary = Path(tmpdir) / "mugi_wifi_enterprise_harness"
        build = subprocess.run(
            [
                *cc, "-std=c11", "-Wall", "-Wextra", *defines,
                "-I", str(ROOT / "tests" / "mugi_fakes"),
                "-I", str(COMP / "include"),
                str(ROOT / "tests" / "mugi_wifi_enterprise_harness.c"),
                str(COMP / "mugi_wifi_enterprise.c"),
                "-o", str(binary),
            ],
            text=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE,
        )
        if build.returncode != 0:
            raise AssertionError(build.stdout + build.stderr)
        return subprocess.run(
            [str(binary)], text=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE
        )


ENABLED = [
    "-DCONFIG_MUGI_WIFI_ENTERPRISE=1",
    '-DCONFIG_MUGI_EAP_SSID="corp-net"',
    '-DCONFIG_MUGI_EAP_USERNAME="alice"',
    '-DCONFIG_MUGI_EAP_PASSWORD="password"',
    '-DCONFIG_MUGI_EAP_SERVER_DOMAIN="radius.example.org"',
    "-DCONFIG_MUGI_EAP_DISABLE_TIME_CHECK=1",
]


class MugiWifiEnterpriseTest(unittest.TestCase):
    def check(self, defines: list[str]) -> None:
        proc = run_harness(defines)
        self.assertEqual(proc.returncode, 0, msg=proc.stdout + proc.stderr)

    def test_peap_setup_and_teardown(self) -> None:
        self.check([*ENABLED, '-DCONFIG_MUGI_EAP_ANON_IDENTITY=""'])

    def test_anonymous_outer_identity(self) -> None:
        self.check([
            *ENABLED, '-DCONFIG_MUGI_EAP_ANON_IDENTITY="anon@example.org"',
            "-DTEST_ANON=1",
        ])

    def test_disabled_build_does_nothing(self) -> None:
        self.check([])

    def test_missing_password_refuses_to_enable(self) -> None:
        defines = [d for d in ENABLED if "EAP_PASSWORD" not in d]
        defines += [
            '-DCONFIG_MUGI_EAP_PASSWORD=""',
            '-DCONFIG_MUGI_EAP_ANON_IDENTITY=""',
            "-DTEST_NOPASS=1",
        ]
        self.check(defines)


if __name__ == "__main__":
    unittest.main()
