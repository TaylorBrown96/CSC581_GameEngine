# 🚀 GitHub Issues & Milestones Bulk Loader

This project provides a Python script to **bulk-create milestones and issues** in a GitHub repository using the **REST API**.  
It reads two JSON files:
- `milestones.json` → defines the milestones (title, description, due date)
- `issues_data.json` → defines the issues (title, body, labels, and milestone assignment)

---

## 📦 Requirements
- Python 3.7+
- `requests` library

Install dependencies:
```bash
pip install requests
```

---

## 🔑 Generating a GitHub Personal Access Token (PAT)

1. Go to your GitHub [Developer Settings → Personal access tokens → Tokens (classic)](https://github.com/settings/tokens).
2. Click **Generate new token** → **Generate new token (classic)**.
3. Give it a descriptive name, like `bulk-issues-script`.
4. Set an **expiration date** (recommended: 30–90 days, or "No expiration" if you prefer).
5. Select scopes:
   - For **private repositories** → check **`repo`**
   - For **public repositories only** → check **`public_repo`**
6. Generate and **copy the token** (starts with `ghp_...`). GitHub will not show it again.

---

## ⚙️ Setting the Token as an Environment Variable

### macOS/Linux
```bash
export GITHUB_TOKEN=ghp_yourTokenHere
```

### Windows PowerShell
```powershell
setx GITHUB_TOKEN "ghp_yourTokenHere"
# Restart the terminal after running this
```

Check it’s set:
```bash
# macOS/Linux
echo $GITHUB_TOKEN

# Windows PowerShell
echo $env:GITHUB_TOKEN
```

---

## 📝 Example Files

### `milestones.json`
```json
[
  {
    "title": "Milestone 1A – Engine Foundations (Team)",
    "description": "Core SDL3 setup, entity, physics, input, collision, scaling. Team submission + docs."
  },
  {
    "title": "Milestone 1B – Game Foundations (Individual)",
    "description": "Individual game using team engine: entities, physics, input, collisions, scaling. Reflection + screenshots."
  }
]
```

### `issues_data.json`
```json
[
  {
    "title": "[Engine] Core Graphics Setup",
    "labels": ["engine"],
    "milestone_title": "Milestone 1A – Engine Foundations (Team)",
    "body": "Establish SDL3 initialization and main loop..."
  },
  {
    "title": "[Game] Implement Required Entities",
    "labels": ["game"],
    "milestone_title": "Milestone 1B – Game Foundations (Individual)",
    "body": "Create static, controllable, and auto-moving entities..."
  }
]
```

---

## ▶️ Running the Script

Run the script and point it to your repo, milestones, and issues files:

```bash
python create_github_issues.py   --repo your-username/CSC581_GameEngine   --milestones milestones.json   --issues issues_data.json
```

### What it does:
1. Creates (or reuses) milestones from `milestones.json`.
2. Ensures labels from `issues_data.json` exist in the repo.
3. Creates all issues and assigns them to the correct milestone.

---

## ✅ Tips
- Always test on a **sandbox repo** before running on your class project.
- You can safely re-run the script; it will reuse existing milestones/labels but may duplicate issues if re-run unchanged.
- For future milestones, just drop in new `milestones.json` and `issues_data.json` files and re-run.

---
