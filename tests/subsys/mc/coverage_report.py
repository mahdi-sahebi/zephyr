# SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
# SPDX-License-Identifier: Apache-2.0
"""Check Micro Core production coverage from Twister's gcovr XML."""

import argparse
import xml.etree.ElementTree as ET

SOURCES = ("subsys/micro_core/comm.c", "subsys/micro_core/msg.c")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("xml", help="Twister coverage/coverage.xml")
    parser.add_argument("--source", action="append", dest="sources",
                        help="Source to check; defaults to comm.c and msg.c")
    parser.add_argument("--min-line-percent", type=float, default=0.0)
    parser.add_argument("--min-branch-percent", type=float, default=0.0)
    args = parser.parse_args()
    root = ET.parse(args.xml).getroot()
    failed = False

    for source in args.sources or SOURCES:
        matches = [item for item in root.findall(".//class")
                   if item.get("filename") == source]
        if len(matches) != 1:
            parser.error(f"expected one coverage entry for {source}, found {len(matches)}")
        lines = matches[0].findall("./lines/line")
        if not lines:
            parser.error(f"no executable lines found for {source}")
        covered = sum(int(line.get("hits", "0")) > 0 for line in lines)
        percent = 100.0 * covered / len(lines)
        branch_rate = matches[0].get("branch-rate")
        if branch_rate is None:
            parser.error(f"no branch coverage found for {source}")
        branch_percent = 100.0 * float(branch_rate)
        print(f"{source}: {covered}/{len(lines)} lines ({percent:.2f}%), "
              f"{branch_percent:.2f}% branches")
        if percent < args.min_line_percent or branch_percent < args.min_branch_percent:
            failed = True

    return 1 if failed else 0


if __name__ == "__main__":
    raise SystemExit(main())
