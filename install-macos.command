#!/bin/bash
set -euo pipefail

# Kick Me local development installer. Runs on the user's Mac, never in GitHub Actions.
SOURCE="$(cd "$(dirname "$0")" && pwd)"
echo "Kick Me macOS Installer"
echo "Source: $SOURCE"
echo ""
echo "Quit Ableton Live and other audio hosts before continuing."
echo "Press Return to install VST3, AU, and standalone app to your user account."
read -r _

install_bundle() {
  local name="$1" destination="$2"; shift 2
  if [[ -d "$SOURCE/$name" ]]; then
    mkdir -p "$(dirname "$destination")"
    # Remove bundles from older builds (Infinati Kick) so Live does not list the plugin twice
    local legacy
    for legacy in "$@"; do rm -rf "$legacy"; done
    if [[ -e "$destination" ]]; then
      rm -rf "$destination"
    fi
    ditto "$SOURCE/$name" "$destination"
    # This is only suitable for your own trusted, unsigned development builds.
    xattr -dr com.apple.quarantine "$destination" 2>/dev/null || true
    echo "Installed $name -> $destination"
  else
    echo "Missing bundle: $name" >&2
    exit 1
  fi
}

install_bundle "KickMe.vst3" "$HOME/Library/Audio/Plug-Ins/VST3/KickMe.vst3" \
  "$HOME/Library/Audio/Plug-Ins/VST3/InfinatiKick.vst3" "$HOME/Library/Audio/Plug-Ins/VST3/Infinati Kick.vst3"
install_bundle "KickMe.component" "$HOME/Library/Audio/Plug-Ins/Components/KickMe.component" \
  "$HOME/Library/Audio/Plug-Ins/Components/InfinatiKick.component" "$HOME/Library/Audio/Plug-Ins/Components/Infinati Kick.component"
install_bundle "KickMe.app" "$HOME/Applications/KickMe.app" \
  "$HOME/Applications/InfinatiKick.app" "$HOME/Applications/Infinati Kick.app"

echo ""
echo "Finished. Restart Ableton Live, enable VST3 system folders, then Option-click Rescan."
echo "Press Return to exit."
read -r _
