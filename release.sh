#!/bin/bash
# dosdoor release script
#
# Usage:
#   ./release.sh <version>
#
# Example:
#   ./release.sh 20260310.1

set -e

if [ $# -ne 1 ]; then
    echo "Usage: $0 <version>"
    echo "Example: $0 20260310.1"
    exit 1
fi

VERSION="$1"

if ! echo "$VERSION" | grep -qE '^[0-9]{8}\.[0-9]+$'; then
    echo "ERROR: Invalid version format: $VERSION"
    echo "Expected: YYYYMMDD.N (e.g., 20260310.1)"
    exit 1
fi

if ! git rev-parse --git-dir > /dev/null 2>&1; then
    echo "ERROR: Not in a git repository"
    exit 1
fi

if ! git diff-index --quiet HEAD --; then
    echo "ERROR: Uncommitted changes"
    git status --short
    exit 1
fi

if git rev-parse "$VERSION" >/dev/null 2>&1; then
    echo "ERROR: Tag $VERSION already exists"
    exit 1
fi

echo "Preparing dosdoor release: $VERSION"

# Update VERSION file
echo "$VERSION" > VERSION

# Commit and tag
git add VERSION
git commit -m "chore(release): $VERSION"
git tag -a "$VERSION" -m "dosdoor $VERSION"

echo ""
echo "Release $VERSION prepared."
echo ""
echo "Next:"
echo "  git push && git push --tags"
