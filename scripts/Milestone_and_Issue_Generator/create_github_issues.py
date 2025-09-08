#!/usr/bin/env python3
# Bulk-create milestones from milestones.json, ensure labels, and create issues assigned by milestone_title.
# Usage:
#   GITHUB_TOKEN=*** python create_github_issues.py --repo your-username/CSC581_GameEngine \
#       --milestones milestones.json --issues issues_data.json
#
# Required scopes: 'repo' (private) or 'public_repo' (public)

import argparse, json, os, sys, time
import requests

API_ROOT = "https://api.github.com"

def gh_headers():
    token = os.getenv("GITHUB_TOKEN")
    if not token:
        print("ERROR: Set GITHUB_TOKEN env var with a Personal Access Token (scope: repo/public_repo).", file=sys.stderr)
        sys.exit(1)
    return {"Authorization": f"token {token}", "Accept": "application/vnd.github+json"}

def ensure_milestone(owner, repo, title, description=None, due_on=None):
    url = f"{API_ROOT}/repos/{owner}/{repo}/milestones"
    headers = gh_headers()
    # fetch existing (open)
    r = requests.get(url, headers=headers, params={"state": "open"}, timeout=30)
    r.raise_for_status()
    for m in r.json():
        if m.get("title") == title:
            print(f"Milestone already exists: {title} (number={m['number']})")
            return m
    payload = {"title": title}
    if description:
        payload["description"] = description
    if due_on:
        payload["due_on"] = due_on  # ISO8601
    r = requests.post(url, headers=headers, json=payload, timeout=30)
    r.raise_for_status()
    m = r.json()
    print(f"Created milestone: {m['title']} (number={m['number']})")
    return m

def ensure_labels(owner, repo, labels):
    url = f"{API_ROOT}/repos/{owner}/{repo}/labels"
    headers = gh_headers()
    r = requests.get(url, headers=headers, timeout=30)
    r.raise_for_status()
    existing = {l['name'] for l in r.json()}
    for lb in labels:
        if lb in existing:
            print(f"Label exists: {lb}")
            continue
        payload = {"name": lb}
        rr = requests.post(url, headers=headers, json=payload, timeout=30)
        if rr.status_code not in (200, 201):
            print(f"WARNING: Could not create label '{lb}' -> {rr.status_code} {rr.text}")
        else:
            print(f"Created label: {lb}")
        time.sleep(0.2)

def create_issue(owner, repo, title, body, labels, milestone_number):
    url = f"{API_ROOT}/repos/{owner}/{repo}/issues"
    headers = gh_headers()
    payload = {
        "title": title,
        "body": body,
        "labels": labels,
        "milestone": milestone_number,
    }
    r = requests.post(url, headers=headers, json=payload, timeout=30)
    if r.status_code not in (200, 201):
        print(f"ERROR creating '{title}': {r.status_code} {r.text}", file=sys.stderr)
    else:
        js = r.json()
        print(f"Created issue #{js['number']}: {title}")
    time.sleep(0.25)

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--repo", required=True, help="owner/repo (e.g., your-username/CSC581_GameEngine)")
    ap.add_argument("--milestones", default="milestones.json", help="Path to milestones JSON")
    ap.add_argument("--issues", default="issues_data.json", help="Path to issues JSON")
    args = ap.parse_args()

    owner, repo = args.repo.split("/", 1)

    # Load milestones & ensure they exist
    with open(args.milestones, "r", encoding="utf-8") as f:
        milestones = json.load(f)

    milestone_map = {}
    for m in milestones:
        title = m["title"]
        desc = m.get("description")
        due_on = m.get("due_on")
        ms = ensure_milestone(owner, repo, title, desc, due_on)
        milestone_map[title] = ms["number"]

    # Load issues
    with open(args.issues, "r", encoding="utf-8") as f:
        issues = json.load(f)

    # Ensure labels from issues
    all_labels = sorted({lb for it in issues for lb in it.get("labels", [])})
    ensure_labels(owner, repo, all_labels)

    # Create issues assigned to the right milestone
    for it in issues:
        title = it["title"]
        body = it.get("body", "")
        labels = it.get("labels", [])
        ms_title = it.get("milestone_title")
        ms_num = milestone_map.get(ms_title)
        if not ms_num:
            print(f"WARNING: Milestone title not found for issue '{title}': {ms_title}", file=sys.stderr)
            continue
        create_issue(owner, repo, title, body, labels, ms_num)

if __name__ == "__main__":
    main()
