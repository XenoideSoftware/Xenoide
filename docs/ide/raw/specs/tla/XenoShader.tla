---- MODULE XenoShader ----
EXTENDS Sequences, FiniteSets, TLC

\* ────────────────────────────────────────────────────────────────────────────
\* CONSTANTS
\* Declare any model-level parameters here (e.g. sets of files, users, etc.)
\* ────────────────────────────────────────────────────────────────────────────

\* (none yet)

\* ────────────────────────────────────────────────────────────────────────────
\* STATE VARIABLES
\* One variable per piece of mutable state the model needs to track.
\* ────────────────────────────────────────────────────────────────────────────

VARIABLES
    appState    \* current application state (one of AppStates below)

\* ────────────────────────────────────────────────────────────────────────────
\* TYPE DEFINITIONS
\* Plain sets used as enumerations — TLA+ has no enum keyword.
\* ────────────────────────────────────────────────────────────────────────────

AppStates == {
    "Idle",
    "Editing",
    "Compiled",
    "CompileError",
    "Debugging",
    "Paused"
}

\* ────────────────────────────────────────────────────────────────────────────
\* TYPE INVARIANT
\* Asserts that every variable holds a value of the expected type.
\* TLC checks this on every reachable state when listed under INVARIANT in .cfg
\* ────────────────────────────────────────────────────────────────────────────

TypeOK ==
    appState \in AppStates

\* ────────────────────────────────────────────────────────────────────────────
\* INITIAL STATE
\* Defines the set of valid starting states for the model.
\* ────────────────────────────────────────────────────────────────────────────

Init ==
    appState = "Idle"

\* ────────────────────────────────────────────────────────────────────────────
\* ACTIONS
\* Each action is a predicate over (current state, next state).
\* Convention: prime (') refers to the value of a variable in the *next* state.
\* Add one action per state transition. Example stub below.
\* ────────────────────────────────────────────────────────────────────────────

\* Example: file opened -> move from Idle (or any state) to Editing
\* FileOpened ==
\*     /\ appState \in {"Idle", "Editing", "Compiled", "CompileError"}
\*     /\ appState' = "Editing"

\* TODO: add one action per event that drives a state transition

\* ────────────────────────────────────────────────────────────────────────────
\* NEXT-STATE RELATION
\* The disjunction of all actions. TLC explores every possible action at each step.
\* ────────────────────────────────────────────────────────────────────────────

Next ==
    \/ FALSE    \* replace FALSE with your actions as you add them

\* ────────────────────────────────────────────────────────────────────────────
\* SPECIFICATION
\* Combines Init and Next into the full temporal formula checked by TLC.
\* ────────────────────────────────────────────────────────────────────────────

Spec == Init /\ [][Next]_appState

====