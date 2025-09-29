
# Original vs. Decentralized P2P — Quick Comparison

Here’s the practical diff between your two flavors:

| Area | **Original P2P (hybrid / listen-server)** | **Decentralized P2P (no authority)** |
|---|---|---|
| Authority | **Lowest peer ID** is authoritative for shared objects (e.g., the ball). | **No single authority.** Every peer simulates everything. |
| What each peer sends | Each peer sends **its own input/state** (e.g., `PADDLE <id> <y>`). The authority also sends **world state** (e.g., `BALL <x y vx vy scoreL scoreR>`). | Every peer sends **both** its input and its **local world state** (e.g., `PADDLE …` and `BALL2 <id clock x y vx vy>` + `SCORE2 …`). |
| Conflict resolution | None needed—authority is the source of truth for shared objects. | **Lamport clock** + **tie-break by peer ID** to pick the latest state; **CRDT (max merge)** for scores. |
| Convergence | Instant: non-authority peers just accept the authority’s state. | **Eventual:** peers smooth toward the latest agreed target (position is eased; velocity adopted immediately). |
| Late join behavior | Tracker reply includes a **snapshot** so new peers connect to existing ones; the authority continues to drive state. | Same snapshot handshake, but new peers begin simulating and converge via incoming `BALL2` updates. |
| Bandwidth | Lower: only one peer broadcasts the heavy shared state. | Higher: **both** peers broadcast state (throttled ~20 Hz in v2). |
| Latency feel | Very stable; remote player sees authoritative corrections (can feel snappy). | More “peer-ish”; small nudges as states reconcile. Tunable via smoothing/send rate. |
| Failure modes | If the authority drops, the match loses its source of truth (you’d promote a new authority). | If one peer drops, the other keeps simulating; once the peer returns, clocks/CRDTs resync. |
| Anti-cheat | Easier—authority can validate. | Harder—no single validator. |
| Sim determinism | Not required; authority dictates. | Helpful but not required; reconciliation handles drift. |
| Code touchpoints | Uses `P2PNode` + `P2PTracker` and messages `PADDLE`, `BALL`. | Uses `P2PNodeV2` + `P2PTrackerV2` and messages `PADDLE`, `BALL2`, `SCORE2`. |

## Message flows at a glance

**Original (listen-server)**  
- Peer A (authority): publishes `BALL …` every frame; both peers publish `PADDLE …`.  
- Peer B: renders with authority’s ball and scores.

**Decentralized (no authority)**  
- Both peers: integrate ball locally each frame.  
- Every ~50 ms send `BALL2 <id clock x y vx vy>` (Lamport clock; tie-break by lower peer ID).  
- Apply newest update on receipt; **smooth position** toward target and **max-merge** scores via `SCORE2`.

## When to use which

- **Original (authority):** competitive games, simpler syncing, lower bandwidth, easier anti-cheat.
- **Decentralized:** extra-credit / researchy setups, resilient when nobody wants to “host,” tolerant to brief disconnects.

## Notes on your implementations

- **Snapshot join:** Both versions use a tracker that replies `ASSIGN <id> <N> <id ep>...` so late joiners immediately connect to current peers (no missed JOIN).  
- **Decentralized tuning:** Current v2 publishes `BALL2` ~20 Hz, adopts velocity immediately, smooths **position only**, and uses speed presets per your latest tuning.

---

*If you want a single binary that can switch between the two demos at runtime, we can add a CLI flag or compile-time option (`-DDEMO_MODE=authority|decentralized`).*
