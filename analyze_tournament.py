#!/usr/bin/env python3
import argparse
import json
import math
import os
from dataclasses import dataclass
from typing import Dict, List, Tuple

import matplotlib.pyplot as plt


@dataclass
class AgentStats:
    agent: str
    games: int = 0
    wins: int = 0
    losses: int = 0
    draws: int = 0
    points_scored: int = 0
    points_allowed: int = 0
    timeouts: int = 0
    crashes: int = 0

    def average_score(self) -> float:
        return self.points_scored / self.games if self.games else 0.0

    def average_allowed(self) -> float:
        return self.points_allowed / self.games if self.games else 0.0

    def average_margin(self) -> float:
        if not self.games:
            return 0.0
        return (self.points_scored - self.points_allowed) / self.games

    def win_rate(self) -> float:
        return self.wins / self.games if self.games else 0.0


@dataclass
class Weights:
    win_points: float = 1.0
    draw_points: float = 0.25
    loss_points: float = -0.5
    margin_weight: float = 0.02  # per point of average margin
    timeout_penalty: float = 1.0
    crash_penalty: float = 1.5


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Analyze Othello tournament JSON and produce advanced leaderboard and plots."
    )
    parser.add_argument(
        "input",
        help="Path to tournament results JSON (as produced by your tournament runner)",
    )
    parser.add_argument(
        "--output-dir",
        default="analysis_out",
        help="Directory to save plots and leaderboard TSV (default: analysis_out)",
    )
    parser.add_argument(
        "--show",
        action="store_true",
        help="Show plots interactively in addition to saving PNGs",
    )
    # Scoring weights
    parser.add_argument("--win-points", type=float, default=Weights.win_points)
    parser.add_argument("--draw-points", type=float, default=Weights.draw_points)
    parser.add_argument("--loss-points", type=float, default=Weights.loss_points)
    parser.add_argument("--margin-weight", type=float, default=Weights.margin_weight)
    parser.add_argument(
        "--timeout-penalty", type=float, default=Weights.timeout_penalty
    )
    parser.add_argument("--crash-penalty", type=float, default=Weights.crash_penalty)

    return parser.parse_args()


def load_json(path: str) -> dict:
    with open(path, "r", encoding="utf-8") as f:
        return json.load(f)


def accumulate_stats(data: dict) -> Dict[str, AgentStats]:
    agents: List[str] = data.get("agents") or []
    stats: Dict[str, AgentStats] = {a: AgentStats(agent=a) for a in agents}

    games: List[dict] = data.get("games") or []
    for g in games:
        black = g["black"]
        white = g["white"]
        b_score = int(g["blackScore"]) if g.get("blackScore") is not None else 0
        w_score = int(g["whiteScore"]) if g.get("whiteScore") is not None else 0
        winner = g.get("winner")  # "BLACK", "WHITE", or maybe "DRAW"
        black_timed_out = bool(g.get("blackTimedOut", False))
        white_timed_out = bool(g.get("whiteTimedOut", False))
        black_crashed = bool(g.get("blackCrashed", False))
        white_crashed = bool(g.get("whiteCrashed", False))

        # Ensure stats exist even if agents list is missing
        if black not in stats:
            stats[black] = AgentStats(agent=black)
        if white not in stats:
            stats[white] = AgentStats(agent=white)

        sb = stats[black]
        sw = stats[white]
        sb.games += 1
        sw.games += 1
        sb.points_scored += b_score
        sb.points_allowed += w_score
        sw.points_scored += w_score
        sw.points_allowed += b_score

        # Failures
        if black_timed_out:
            sb.timeouts += 1
        if white_timed_out:
            sw.timeouts += 1
        if black_crashed:
            sb.crashes += 1
        if white_crashed:
            sw.crashes += 1

        # Outcome
        if winner == "BLACK":
            sb.wins += 1
            sw.losses += 1
        elif winner == "WHITE":
            sw.wins += 1
            sb.losses += 1
        else:
            # Some JSON use explicit draw marker, otherwise infer on equal scores
            if b_score == w_score:
                sb.draws += 1
                sw.draws += 1
            else:
                # Fallback: use score to determine winner
                if b_score > w_score:
                    sb.wins += 1
                    sw.losses += 1
                elif w_score > b_score:
                    sw.wins += 1
                    sb.losses += 1
                else:
                    sb.draws += 1
                    sw.draws += 1

    return stats


def compute_advanced_scores(
    stats: Dict[str, AgentStats], weights: Weights
) -> Dict[str, float]:
    scores: Dict[str, float] = {}
    for agent, s in stats.items():
        base = (
            s.wins * weights.win_points
            + s.draws * weights.draw_points
            + s.losses * weights.loss_points
        )
        margin_bonus = s.average_margin() * weights.margin_weight
        penalties = (
            s.timeouts * weights.timeout_penalty + s.crashes * weights.crash_penalty
        )
        scores[agent] = base + margin_bonus - penalties
    return scores


def print_leaderboard(
    stats: Dict[str, AgentStats], adv_scores: Dict[str, float]
) -> List[Tuple[str, AgentStats, float]]:
    ranked = sorted(
        [(a, stats[a], adv_scores[a]) for a in stats.keys()],
        key=lambda t: t[2],
        reverse=True,
    )

    # Header
    print("\nAdvanced Leaderboard (higher is better)")
    print(
        f"{'Rank':>4}  {'Agent':<20} {'Score':>8}  {'W':>3} {'L':>3} {'D':>3}  {'WR%':>6}  {'AvgMargin':>10}  {'TO':>3} {'CR':>3}"
    )
    for i, (agent, s, score) in enumerate(ranked, start=1):
        print(
            f"{i:>4}  {agent:<20} {score:>8.3f}  {s.wins:>3} {s.losses:>3} {s.draws:>3}  "
            f"{(100*s.win_rate()):>6.1f}  {s.average_margin():>10.3f}  {s.timeouts:>3} {s.crashes:>3}"
        )
    return ranked


def ensure_outdir(path: str) -> None:
    os.makedirs(path, exist_ok=True)


def save_leaderboard_tsv(
    ranked: List[Tuple[str, AgentStats, float]], outdir: str
) -> str:
    path = os.path.join(outdir, "leaderboard.tsv")
    with open(path, "w", encoding="utf-8") as f:
        f.write(
            "rank\tagent\tscore\twins\tlosses\tdraws\twin_rate\tavg_margin\ttimeouts\tcrashes\n"
        )
        for i, (agent, s, score) in enumerate(ranked, start=1):
            f.write(
                f"{i}\t{agent}\t{score:.6f}\t{s.wins}\t{s.losses}\t{s.draws}\t{(100*s.win_rate()):.3f}\t{s.average_margin():.6f}\t{s.timeouts}\t{s.crashes}\n"
            )
    return path


def plot_advanced_score_bar(
    ranked: List[Tuple[str, AgentStats, float]], outdir: str
) -> str:
    agents = [a for a, _, _ in ranked]
    scores = [sc for _, __, sc in ranked]

    plt.figure(figsize=(10, 5))
    bars = plt.bar(agents, scores, color="#4C78A8")
    plt.title("Advanced Score by Agent")
    plt.ylabel("Advanced Score")
    plt.grid(axis="y", linestyle=":", alpha=0.4)
    plt.xticks(rotation=30, ha="right")
    for b, s in zip(bars, scores):
        plt.text(
            b.get_x() + b.get_width() / 2,
            b.get_height(),
            f"{s:.2f}",
            ha="center",
            va="bottom",
            fontsize=9,
        )
    path = os.path.join(outdir, "advanced_score_bar.png")
    plt.tight_layout()
    plt.savefig(path)
    return path


def plot_results_stacked(stats: Dict[str, AgentStats], outdir: str) -> str:
    agents = list(stats.keys())
    wins = [stats[a].wins for a in agents]
    draws = [stats[a].draws for a in agents]
    losses = [stats[a].losses for a in agents]

    plt.figure(figsize=(10, 6))
    p1 = plt.bar(agents, wins, label="Wins", color="#4C78A8")
    p2 = plt.bar(agents, draws, bottom=wins, label="Draws", color="#72B7B2")
    bottom_for_losses = [w + d for w, d in zip(wins, draws)]
    p3 = plt.bar(
        agents, losses, bottom=bottom_for_losses, label="Losses", color="#F58518"
    )
    plt.title("Results Composition per Agent")
    plt.ylabel("Games")
    plt.legend()
    plt.xticks(rotation=30, ha="right")
    plt.grid(axis="y", linestyle=":", alpha=0.4)
    path = os.path.join(outdir, "results_stacked_bar.png")
    plt.tight_layout()
    plt.savefig(path)
    return path


def plot_winrate_vs_margin(
    stats: Dict[str, AgentStats], adv_scores: Dict[str, float], outdir: str
) -> str:
    agents = list(stats.keys())
    x = [100 * stats[a].win_rate() for a in agents]
    y = [stats[a].average_margin() for a in agents]
    sizes = [80 + 40 * (stats[a].timeouts + stats[a].crashes) for a in agents]
    colors = [adv_scores[a] for a in agents]

    plt.figure(figsize=(8, 6))
    sc = plt.scatter(
        x, y, s=sizes, c=colors, cmap="viridis", alpha=0.85, edgecolors="black"
    )
    for i, a in enumerate(agents):
        plt.text(x[i] + 0.3, y[i], a, fontsize=9, va="center")
    plt.colorbar(sc, label="Advanced Score")
    plt.title("Win Rate vs Avg Margin (size ~ failures)")
    plt.xlabel("Win Rate (%)")
    plt.ylabel("Average Margin (points)")
    plt.grid(True, linestyle=":", alpha=0.4)
    path = os.path.join(outdir, "winrate_vs_margin.png")
    plt.tight_layout()
    plt.savefig(path)
    return path


def main() -> None:
    args = parse_args()

    weights = Weights(
        win_points=args.win_points,
        draw_points=args.draw_points,
        loss_points=args.loss_points,
        margin_weight=args.margin_weight,
        timeout_penalty=args.timeout_penalty,
        crash_penalty=args.crash_penalty,
    )

    data = load_json(args.input)
    stats = accumulate_stats(data)
    adv_scores = compute_advanced_scores(stats, weights)

    ranked = print_leaderboard(stats, adv_scores)

    ensure_outdir(args.output_dir)
    tsv_path = save_leaderboard_tsv(ranked, args.output_dir)

    # Plots
    paths = []
    paths.append(plot_advanced_score_bar(ranked, args.output_dir))
    paths.append(plot_results_stacked(stats, args.output_dir))
    paths.append(plot_winrate_vs_margin(stats, adv_scores, args.output_dir))

    print("\nSaved:")
    print(f"- Leaderboard TSV: {tsv_path}")
    for p in paths:
        print(f"- Plot: {p}")

    if args.show:
        plt.show()
    else:
        # Close figures to free memory in non-interactive usage
        plt.close("all")


if __name__ == "__main__":
    main()
