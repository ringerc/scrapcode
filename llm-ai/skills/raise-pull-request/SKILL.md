---
name: raise-pr
description: >-
    Push the current branch and open a pull or draft pull request on GitHub. Use when the user asks to
    raise, open, or draft a PR for a completed change set. Follows the repo's PR template if one
    is present, keeps the body succinct, and covers companion-PR callouts if the user mentions them.
user-invocable: true
---

# Raising a pull request

Use this skill when the user asks you to open, raise, or draft a PR for work already committed on a branch.

## Prerequisites

- Working directory is a clean git repo with commits on a non-default branch.
- `GH_TOKEN` or `GITHUB_TOKEN` is set (Copilot CLI sets this by default).
- The remote is a GitHub `https://github.com/OWNER/REPO` URL.
- (for a non-draft PR): Locally-excecutable tests have been run, or the user
  has approved raising a PR without running them.

## Steps

### 1. Confirm the change set

Check `git status -sb` and `git log --oneline @{u}..` (or `origin/DEFAULT..HEAD` if no upstream). If there are uncommitted changes, ask the user whether to include them before proceeding.

### 2. Push the branch

If you are a native Claude Code agent, pushing should just work.

If you are a Claude agent in a GitHub Copilot container, the default git remote
uses HTTPS and CLI terminals disable password prompts, so `git push` fails
silently. Push with the token embedded in the URL:

```bash
git push -u "https://x-access-token:${GH_TOKEN}@github.com/OWNER/REPO" BRANCH
```

Never write the token to a file. Do not add it as a stored remote.

### 3. Read the PR template

If `.github/PULL_REQUEST_TEMPLATE.md` or `.github/pull_request_template.md` exists, mirror its section headings exactly. Common headings: What, Why, How, Test, Context.

If no template, use: Summary, Motivation, Changes, Testing, Notes, References.

### 4. Draft the body

Follow these rules unless the user overrides them:

- **Write for the reviewer not the author.** The PR is for the reviewer; include what they need to know, but keep it succinct and omit what they don't need or can easily determine from the code or the diff.
- **Short sentences.** Plain language. No marketing tone. Use paragraph breaks, no walls of text.
- **Simple punctuation.** One idea per bullet. If a bullet needs a parenthetical aside, a comma-chain of three or more items, or an em-dash to smuggle in a second clause, split it into its own sentence or a nested sub-bullet instead. This applies especially to list-heavy sections (How/Changes) where it's tempting to compress detail into one dense line per change.

  Bad (comma-chain + parenthetical, hard to scan):
  > Baked into templates (no longer configurable): Service port lists, Istio sidecar injection (always on), Prometheus remote-write source pinning (always on), security contexts (fsGroup/runAsUser 65532, with the existing RHOS carve-out).

  Good (one fact per line):
  > Several stanzas move out of values and into the templates, so they are no longer configurable:
  > - Service port lists
  > - Istio sidecar injection (always on now)
  > - Prometheus remote-write source pinning (always on now)
  > - Security contexts (fsGroup/runAsUser 65532, keeping the existing RHOS carve-out) Hyphens not proper dashes (sigh). Avoid parentheticals where a separate sentence would be clearer.
- **Focus on the result and its effects**, not the journey. Do not narrate what was tried and rejected.
- **No word-wrap.** Let the renderer wrap.
- **No filename lists, commit hashes, or references to un-versioned planning docs** unless a reviewer genuinely needs them to act.
- **Omit references to untracked working documents and plans.** Only refer to what the reader will be able to see. No phase numbers, no section refs, etc.
- **Hyperlink** Jira tickets, related PRs, and issue numbers. Jira: `[TICKET-123](https://enterprisedb.atlassian.net/browse/TICKET-123)`. Cross-repo PR: `owner/repo#123`.
- **Companion PRs**: if the user mentions changes required in other repos before merging, call them out near the top under a bold heading. Otherwise put them hyperlinked in the context/references.
- **Draft by default.** Prefer to create draft PRs. If you think the PR is straightforward and mergeable, ask the user if they want to make it draft or not.

For PRs whose target branch is outside the 'EnterpriseDB' or 'ringerc' GitHub organisations, do not "leak" references to Jira ticket numbers, "upm-xxx", "edb-xxx", "beacon-xxx" names into the PR body. Treat them as upstream contributions without reference to any internal work that may relate to them. If you think a particular PR needs such references, prompt the user.

Avoid these words and patterns:
- "comprehensive", "seamless", "robust", "leverage", "streamline", "lift", "surface", "load-bearing", "blast-radius", "footgun", "belt-and-suspenders", "smoking gun", "seam"/"seamless", "pin"/"pinned", "gate"/"gated"; use these if their very specific meaning is part of established technical vocabulary (e.g. version pinning) but otherwise prefer natural language.
- "honestly", "the honest truth"; they're implied
- "This PR does X. This PR also does Y." Use bullets.
- Emoji unless the repo already uses them in PR bodies.

### 5. Open the PR

Prefer `gh pr create` if `gh` is on PATH and authenticated. Otherwise use the REST API directly. Write the body to a temp file first — passing multi-line JSON on the command line breaks quoting.

```bash
python3 -c "import json; body=open('/tmp/pr-body.md').read(); print(json.dumps({
  'title': 'TICKET-123: short imperative summary',
  'head':  'BRANCH',
  'base':  'main',
  'body':  body,
  'draft': True,
}))" > /tmp/pr.json

curl -sS -X POST \
  -H "Authorization: bearer $GH_TOKEN" \
  -H "Accept: application/vnd.github+json" \
  https://api.github.com/repos/OWNER/REPO/pulls \
  -d @/tmp/pr.json \
  | python3 -c "import json,sys; d=json.load(sys.stdin); print(d.get('html_url') or d)"
```

Clean up the temp files after.

### 6. Report the PR URL

Reply with just the PR URL and a one-line note on companion-PR blockers if any. Do not restate the body.

## PR body template (template-free repos)

```markdown
## Summary

One or two sentences on what this changes and why. Link the ticket.

**Companion PRs required before merge:** (omit this stanza if not required)
- `other-repo` — one line on what needs to change there.

## Motivation

Provide the key information the reviewer needs to understand about why this
PR is needed. What it fixes/improves, who it helps. Keep it succinct. Link to
bug tickets, feature requests etc.

## Changes

- Bullet per relevant, significant user-visible change. Focus on behaviour, not implementation.
- Note any breaking changes explicitly.

## Testing

- High level overview of the test cover added if any
- High level overview of test passes; more detail on any that failed, otherwise keep it short.
- Any tests NOT run that should be - e.g. those that require CI workflow approval
- Repro steps where appropriate

## Context

- Jira: [TICKET-123](https://enterprisedb.atlassian.net/browse/TICKET-123)
- Related PRs or issues, if any.

Mention that this PR was drafted an LLM agent on my direct instructions.
```

If there's long content that is materially relevant and important, which does not re-state what
is in the diffs, use `<detail/>` `<summary/>` blocks.

## Common pitfalls

- **Silent push failure.** `git push` with no token in the URL returns `fatal: could not read Username`. Always embed the token.
- **PR body contains a shell substitution.** Never `curl -d "$(cat body.md)"` — quoting eats backticks and `$`. Use `curl -d @file`.
- **Draft flag ignored.** The REST API needs `"draft": true` as a JSON boolean, not the string `"true"`.
- **Wrong base branch.** Check the repo default branch with `git remote show origin | grep HEAD` if unsure.
- **Companion PRs omitted.** If the user mentioned cross-repo work, always list it in the body even if the companion PRs don't exist yet.
