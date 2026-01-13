# Change Log

## [2026-01-13] Crash Log Analysis & Fix
- **FIX**: `selfdrive/car/toyota/interface.py` - Resolved `controlsd` crash by adding exception handling for missing `ToyotaEnforceStockLongitudinal` parameter key.
- **NOTE**: Identified Panda GPS initialization failure (12s after crash) as potential secondary issue.
- **REPORT**: Detailed analysis available in `docs/CRASH_REPORT_2026-01-13.md`.
