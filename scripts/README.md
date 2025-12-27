# Scripts

Utility scripts for EcoSim development and maintenance.

## Available Scripts

### `update-gene-counts.sh`

Automatically updates gene count references in documentation files based on the current source code.

**Purpose:** Keeps documentation in sync with the actual gene count in `UniversalGenes.hpp`.

**Usage:**

```bash
# Update all documentation files
./scripts/update-gene-counts.sh

# Dry run - show what would change without modifying files
./scripts/update-gene-counts.sh -n

# Show help
./scripts/update-gene-counts.sh -h
```

**How it works:**
1. Counts active (non-deprecated) genes in `include/genetics/defaults/UniversalGenes.hpp`
2. Searches documentation files in `docs/` for outdated gene counts
3. Replaces old counts with the current total

**Example output:**

```
Gene Count Update Script
========================

Source: UniversalGenes.hpp
Total active genes: 67
Deprecated genes: 0

Updating documentation...

  Updated: docs/technical/genetics/gene-reference.md (58 → 67)

========================
Done! Updated 1 files.

Total gene count: 67
```

---

## Adding New Scripts

When adding new scripts to this directory:

1. **Use descriptive names** - e.g., `validate-prefabs.sh`, `generate-docs.sh`
2. **Include usage comments** - Add help text at the top of the script
3. **Update this README** - Document the new script in this file
4. **Make executable** - `chmod +x scripts/your-script.sh`

### Script Template

```bash
#!/bin/bash
#
# Script Name - Brief description
# 
# Purpose:
#   What this script does and why
#
# Usage:
#   ./scripts/your-script.sh [options]
#
# Options:
#   -n  Dry run
#   -h  Show help
#

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

# Your script logic here
```

---

## Future Script Ideas

Potential scripts that could be added:

- **`validate-docs.sh`** - Check for broken links and missing references in documentation
- **`generate-gene-docs.sh`** - Auto-generate gene reference documentation from source
- **`check-prefab-balance.sh`** - Validate creature prefab gene values
- **`count-loc.sh`** - Count lines of code by category
