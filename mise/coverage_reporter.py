#!/usr/bin/env python3
"""Aggregate code coverage data and emit text / CSV / HTML reports.

Zero-dependency: only the Python standard library is used.

Inputs
------
* ``llvm-cov``: one or more JSON reports produced by ``llvm-cov export``
  (one per instrumented test executable), passed as positional arguments.
* ``gcov``: a directory (``--gcov-dir``) containing ``.gcov.json.gz`` files
  produced by ``gcov -j`` for every instrumented translation unit.

The reporter filters results to project sources under ``src/`` and merges
coverage across every provided input, so Debug and Release runs (and multiple
test executables) are combined into a single union report.
"""

import argparse
import csv
import gzip
import html
import json
import os
import pathlib
import sys

PROJECT_ROOT = pathlib.Path(__file__).resolve().parent.parent
SRC_ROOT = PROJECT_ROOT / "src"


def source_under_project(path):
    """Return True when *path* points to a project source under src/."""
    path = os.path.normpath(path)
    if not path.startswith(str(SRC_ROOT) + os.sep):
        return False
    for excluded in ("/build/", "/build-tidy/", "/conan/"):
        if excluded in path:
            return False
    return True


def relative_source(path):
    path = os.path.normpath(path)
    if path.startswith(str(SRC_ROOT) + os.sep):
        return path[len(str(SRC_ROOT)) + 1:]
    return path


def merge_file_set(aggregate, path, total, covered):
    entry = aggregate.setdefault(relative_source(path), [set(), set()])
    entry[0].update(total)
    entry[1].update(covered)


def parse_llvm_cov_json(json_path):
    """Read an ``llvm-cov export`` JSON file into per-file line sets."""
    with open(json_path) as handle:
        payload = json.load(handle)
    result = {}
    for data in payload.get("data", []):
        for file_info in data.get("files", []):
            path = file_info.get("filename", "")
            if not source_under_project(path):
                continue
            total = set()
            covered = set()
            for segment in file_info.get("segments", []):
                if len(segment) < 4:
                    continue
                line, _, count, has_count = segment[0], segment[1], segment[2], segment[3]
                if not has_count:
                    continue
                total.add(line)
                if count > 0:
                    covered.add(line)
            merge_file_set(result, path, total, covered)
    return result


def parse_gcov_json(json_gz_path):
    """Read a ``.gcov.json.gz`` file into per-file line sets."""
    with gzip.open(json_gz_path, "rt", encoding="utf-8") as handle:
        payload = json.load(handle)
    result = {}
    for file_info in payload.get("files", []):
        path = file_info.get("file", "")
        if not source_under_project(path):
            continue
        total = set()
        covered = set()
        for line_info in file_info.get("lines", []):
            line = line_info.get("line_number")
            if line is None:
                continue
            total.add(line)
            if line_info.get("count", 0) > 0:
                covered.add(line)
        merge_file_set(result, path, total, covered)
    return result


def collect_llvm_cov(json_files):
    aggregate = {}
    for json_path in json_files:
        for rel, (total, covered) in parse_llvm_cov_json(json_path).items():
            entry = aggregate.setdefault(rel, [set(), set()])
            entry[0].update(total)
            entry[1].update(covered)
    return aggregate


def collect_gcov(gcov_dir):
    aggregate = {}
    gcov_dir = pathlib.Path(gcov_dir)
    if not gcov_dir.is_dir():
        print("Error: gcov data directory not found: {}".format(gcov_dir), file=sys.stderr)
        sys.exit(1)
    json_files = sorted(gcov_dir.rglob("*.gcov.json.gz"))
    if not json_files:
        print("Error: no .gcov.json.gz files found under {}".format(gcov_dir), file=sys.stderr)
        sys.exit(1)
    for json_path in json_files:
        for rel, (total, covered) in parse_gcov_json(json_path).items():
            entry = aggregate.setdefault(rel, [set(), set()])
            entry[0].update(total)
            entry[1].update(covered)
    return aggregate


def build_stats(aggregate):
    stats = []
    for rel, (total_lines, covered_lines) in aggregate.items():
        total = len(total_lines)
        covered = len(covered_lines)
        percent = (covered / total * 100.0) if total else 0.0
        stats.append({"file": rel, "lines": total, "covered": covered, "percent": percent})
    stats.sort(key=lambda s: s["percent"])
    totals = {
        "lines": sum(s["lines"] for s in stats),
        "covered": sum(s["covered"] for s in stats),
    }
    totals["percent"] = (totals["covered"] / totals["lines"] * 100.0) if totals["lines"] else 0.0
    return stats, totals


def emit_text(stats, totals):
    header = "{:<56} {:>8} {:>8} {:>10}".format("File", "Lines", "Covered", "Coverage%")
    print(header)
    print("-" * len(header))
    for s in stats:
        print("{:<56} {:>8} {:>8} {:>9.2f}%".format(
            s["file"][:56], s["lines"], s["covered"], s["percent"]))
    print("-" * len(header))
    print("{:<56} {:>8} {:>8} {:>9.2f}%".format(
        "TOTAL", totals["lines"], totals["covered"], totals["percent"]))


def emit_csv(output_dir, stats, totals):
    output_dir.mkdir(parents=True, exist_ok=True)
    csv_path = output_dir / "coverage.csv"
    with open(csv_path, "w", newline="") as handle:
        writer = csv.writer(handle)
        writer.writerow(["file", "lines", "covered", "coverage_percent"])
        for s in stats:
            writer.writerow([s["file"], s["lines"], s["covered"], round(s["percent"], 2)])
        writer.writerow(["TOTAL", totals["lines"], totals["covered"], round(totals["percent"], 2)])
    print("CSV report written to {}".format(csv_path))


def emit_html(output_dir, stats, totals):
    html_dir = output_dir / "html"
    html_dir.mkdir(parents=True, exist_ok=True)
    index_path = html_dir / "index.html"

    rows = []
    for s in stats:
        width = min(s["percent"], 100.0)
        rows.append(
            "<tr>"
            "<td>{}</td><td>{}</td><td>{}</td><td>{:.2f}%</td>"
            "<td><div class=\"bar\"><div class=\"fill\" style=\"width:{:.2f}%\"></div></div></td>"
            "</tr>".format(html.escape(s["file"]), s["lines"], s["covered"], s["percent"], width)
        )

    total_width = min(totals["percent"], 100.0)
    document = """<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="utf-8">
<title>Xenoide Coverage Report</title>
<style>
body {{ font-family: sans-serif; margin: 2rem; color: #222; }}
h1 {{ font-size: 1.4rem; }}
table {{ border-collapse: collapse; width: 100%; }}
th, td {{ padding: 0.4rem 0.8rem; border-bottom: 1px solid #ddd; text-align: left; }}
th {{ background: #f4f4f4; }}
.bar {{ background: #eee; border-radius: 4px; width: 160px; }}
.fill {{ background: #3a7; height: 14px; border-radius: 4px; }}
tr.total td {{ font-weight: bold; background: #fafafa; }}
</style>
</head>
<body>
<h1>Xenoide Coverage Report</h1>
<p>TOTAL: {total_lines} lines, {total_covered} covered ({total_percent:.2f}%)</p>
<table>
<thead><tr><th>File</th><th>Lines</th><th>Covered</th><th>Coverage</th><th></th></tr></thead>
<tbody>
{rows}
<tr class="total"><td>TOTAL</td><td>{total_lines}</td><td>{total_covered}</td><td>{total_percent:.2f}%</td><td><div class="bar"><div class="fill" style="width:{total_width:.2f}%"></div></div></td></tr>
</tbody>
</table>
</body>
</html>
""".format(
        rows="\n".join(rows),
        total_lines=totals["lines"],
        total_covered=totals["covered"],
        total_percent=totals["percent"],
        total_width=total_width,
    )
    with open(index_path, "w") as handle:
        handle.write(document)
    print("HTML report written to {}".format(index_path))


def main():
    parser = argparse.ArgumentParser(description="Aggregate and report code coverage")
    parser.add_argument("--tool", required=True, choices=["llvm-cov", "gcov"])
    parser.add_argument("--export", required=True, choices=["text", "html", "csv"])
    parser.add_argument("--output-dir", required=True, help="Directory for report files")
    parser.add_argument("--check", type=float, default=None,
                        help="Minimum total line coverage percentage; exits 1 when below")
    parser.add_argument("--gcov-dir", default=None,
                        help="Directory containing .gcov.json.gz files (gcov tool only)")
    parser.add_argument("json_files", nargs="*", help="llvm-cov export JSON files")
    args = parser.parse_args()

    if args.tool == "llvm-cov":
        if not args.json_files:
            print("Error: no llvm-cov export JSON files provided", file=sys.stderr)
            sys.exit(1)
        aggregate = collect_llvm_cov(args.json_files)
    else:
        if not args.gcov_dir:
            print("Error: --gcov-dir is required for gcov", file=sys.stderr)
            sys.exit(1)
        aggregate = collect_gcov(args.gcov_dir)

    if not aggregate:
        print("Error: no project sources found in the coverage data", file=sys.stderr)
        sys.exit(1)

    stats, totals = build_stats(aggregate)
    output_dir = pathlib.Path(args.output_dir)

    if args.export == "text":
        emit_text(stats, totals)
    elif args.export == "csv":
        emit_csv(output_dir, stats, totals)
    else:
        emit_html(output_dir, stats, totals)

    if args.check is not None:
        if totals["percent"] < args.check:
            print("Coverage check FAILED: {:.2f}% < required {:.2f}%".format(
                totals["percent"], args.check))
            sys.exit(1)
        print("Coverage check PASSED: {:.2f}% >= required {:.2f}%".format(
            totals["percent"], args.check))
    sys.exit(0)


if __name__ == "__main__":
    main()