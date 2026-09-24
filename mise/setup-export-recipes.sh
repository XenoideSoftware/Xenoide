#!/bin/bash
set -e

packages_dir="conan/packages"
for pkg in $(ls -1 "$packages_dir" | sort); do
    pkg_dir="$packages_dir/$pkg"
    conanfile="$pkg_dir/conanfile.py"
    conandata="$pkg_dir/conandata.yml"

    if [ ! -f "$conanfile" ]; then
        continue
    fi

    if grep -qP '^\s*version\s*=' "$conanfile"; then
        echo "==> Exporting $pkg ($pkg_dir)"
        conan export "$pkg_dir"
    else
        versions=()
        if [ -f "$conandata" ]; then
            in_sources=false
            while IFS= read -r line; do
                stripped=$(echo "$line" | sed -E 's/^[[:space:]]+//; s/[[:space:]]+$//')
                if echo "$stripped" | grep -q '^sources:'; then
                    in_sources=true
                    continue
                fi
                if [ "$in_sources" = true ]; then
                    if echo "$line" | grep -qP '^\s{2}[^ ]' && ! echo "$line" | grep -qP '^\s{4}'; then
                        version=$(echo "$line" | sed -nE 's/^\s*["'\'']?([^"'\''":]+)["'\'']?\s*:/\1/p')
                        if [ -n "$version" ]; then
                            versions+=("$version")
                        fi
                    elif [ -n "$stripped" ] && ! echo "$line" | grep -qP '^\s' && ! echo "$stripped" | grep -q '^#'; then
                        in_sources=false
                    fi
                fi
            done < "$conandata"
        fi

        if [ ${#versions[@]} -gt 0 ]; then
            for v in "${versions[@]}"; do
                echo "==> Exporting $pkg/$v ($pkg_dir)"
                conan export "$pkg_dir" --version "$v"
            done
        else
            echo "==> Exporting $pkg ($pkg_dir)"
            conan export "$pkg_dir"
        fi
    fi
done