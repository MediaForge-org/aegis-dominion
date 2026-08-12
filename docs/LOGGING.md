# Logging

`logging::Logger` provides DEBUG, INFO, WARNING and ERROR levels, threshold filtering and synchronized stream output. Warning and error messages append file, line and function through `std::source_location`. The process-wide `logging::log()` writes to `std::clog`; tests can construct a logger over any `std::ostream`.

Application startup, game launches, map failures, asset fallbacks and fatal exceptions use the logger. Tests may continue to print their summaries directly because those are test-runner output, not runtime diagnostics.
