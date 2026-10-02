# TLA+ CLI Reference

CLI equivalents for TLA+ Toolbox GUI actions using `tla2tools.jar`.

## CLI Commands

| Toolbox Action | CLI Command |
|---|---|
| **Translate PlusCal** (`File > Translate PlusCal`) | `java -cp tla2tools.jar pcal.trans wire.tla` |
| **Run model checker** (`TLC Model Checker > Run`) | `java -cp tla2tools.jar tlc2.TLC -config wire.cfg wire.tla` |
| **Behavior spec = "Temporal Formula" → `Spec`** | In `wire.cfg`: add `SPECIFICATION Spec` |
| **Add invariant** (e.g. `NoOverdrafts`) | In `wire.cfg`: add `INVARIANT NoOverdrafts` |
| **Scratch: "No behavior spec"** | In `scratch.cfg`: omit `SPECIFICATION` line; add `-deadlock` flag |
| **Scratch: "Evaluate Constant Expression"** | In `scratch.tla`: use `PrintT(Eval)` in an `Init` predicate; set `INIT Init` / `NEXT Next` in cfg |
| **Run scratch model** | `java -cp tla2tools.jar tlc2.TLC -deadlock -config scratch.cfg scratch.tla` |

## Config File Examples

### `wire.cfg` — invariant check

```
SPECIFICATION Spec
INVARIANT NoOverdrafts
```

### `scratch.cfg` — expression evaluator

```
INIT Init
NEXT Next
```

### `scratch.tla` — expression evaluator

```tla
---- MODULE scratch ----
EXTENDS Integers, TLC, Sequences
Eval == (* your expression here *)
Init == PrintT(Eval)
Next == FALSE
====
```

Run with:

```bash
java -cp tla2tools.jar tlc2.TLC -deadlock -config scratch.cfg scratch.tla
```

> The `-deadlock` flag suppresses the "no next state" error when `Next == FALSE`,
> which is what the Toolbox silently does for "no behavior spec" models.
