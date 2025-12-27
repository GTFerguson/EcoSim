#!/bin/bash
#
# Gene Count Auto-Update Script
# Updates gene count references in documentation files based on source code.
#
# Usage:
#   ./scripts/update-gene-counts.sh        # Update all files
#   ./scripts/update-gene-counts.sh -n     # Dry run (show what would change)
#

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

# Source file - UniversalGenes contains ALL genes (PlantGenes is legacy/deprecated)
UNIVERSAL_GENES="$PROJECT_ROOT/include/genetics/defaults/UniversalGenes.hpp"

DRY_RUN=false

while getopts "nh" opt; do
    case $opt in
        n) DRY_RUN=true ;;
        h)
            echo "Usage: $0 [-n] [-h]"
            echo "  -n  Dry run (show what would change)"
            echo "  -h  Show this help"
            exit 0
            ;;
        *) exit 1 ;;
    esac
done

echo "Gene Count Update Script"
echo "========================"
echo ""

# Count genes - use awk to properly handle deprecated annotations on previous line
# Pattern: static constexpr const char* GENE_NAME = "gene_id";
# Skip if previous line contains [[deprecated
TOTAL_COUNT=$(awk '
    /\[\[deprecated/ { deprecated=1; next }
    /static constexpr const char\*.*=.*"[a-z_]+"/ {
        if (!deprecated) count++
        deprecated=0
    }
    { if (!/static constexpr/) deprecated=0 }
    END { print count }
' "$UNIVERSAL_GENES")

DEPRECATED_COUNT=$(grep -c '\[\[deprecated' "$UNIVERSAL_GENES" 2>/dev/null || echo 0)

echo "Source: UniversalGenes.hpp"
echo "Total active genes: $TOTAL_COUNT"
echo "Deprecated genes: $DEPRECATED_COUNT"
echo ""

# Known old total counts to replace (values > 50 that aren't the current total)
OLD_COUNTS=(67 74 88 89 101)

echo "Updating documentation..."
echo ""

UPDATED=0

for old in "${OLD_COUNTS[@]}"; do
    [[ "$old" -eq "$TOTAL_COUNT" ]] && continue
    
    # Find files containing "NN genes" pattern
    while IFS= read -r file; do
        [[ -z "$file" ]] && continue
        rel="${file#$PROJECT_ROOT/}"
        if [[ "$DRY_RUN" == true ]]; then
            echo "  Would update: $rel ($old → $TOTAL_COUNT)"
        else
            sed -i.bak "s/\b${old} genes\b/${TOTAL_COUNT} genes/g" "$file"
            rm -f "${file}.bak"
            echo "  Updated: $rel ($old → $TOTAL_COUNT)"
        fi
        UPDATED=$((UPDATED + 1))
    done < <(grep -rl "\b${old} genes\b" "$PROJECT_ROOT/docs" 2>/dev/null || true)
done

echo ""
echo "========================"
if [[ "$DRY_RUN" == true ]]; then
    echo "Dry run complete. Would update $UPDATED files."
else
    [[ $UPDATED -gt 0 ]] && echo "Done! Updated $UPDATED files." || echo "All counts already up to date."
fi
echo ""
echo "Total gene count: $TOTAL_COUNT"
