import os

import requests

from write_appcast import get_latest_git_tag

version = get_latest_git_tag()
if version is None:
    raise Exception("No git tag found")

repo_owner = "christofmuc"
repo_name = "KnobKraft-orm"
access_token = os.getenv('GITHUB_TOKEN')

# Markdown-formatted release notes
with open(os.path.join("release_notes", f"{version}.md"), 'r') as file:
    release_notes = file.read()

# Create a release payload with the release notes
release_payload = {
    "tag_name": version,
    "name": f"{version}",
    "body": release_notes,
    "draft": False,
    "prerelease": False
}

# Create the release via GitHub API
url = f"https://api.github.com/repos/{repo_owner}/{repo_name}/releases"
headers = {
    "Authorization": f"Bearer {access_token}",
    "Accept": "application/vnd.github.v3+json"
}

# Get existing releases
url_existing_releases = f"https://api.github.com/repos/{repo_owner}/{repo_name}/releases"
existing_releases = requests.get(url_existing_releases, headers=headers).json()

# Check if the release for this version already exists
existing_release = next((release for release in existing_releases if release['tag_name'] == version), None)

if not existing_release:
    print("Creating a new release...")
    response = requests.post(url, json=release_payload, headers=headers)
    if response.status_code == 201:
        print("Release created successfully.")
    else:
        print("Failed to create the release. Response status:", response.status_code)
else:
    print("Found release {version}, skipping creation!")
