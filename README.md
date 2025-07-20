# OBS Countdown Timers Plugin

<p align="center">
<image height="400px" src="/images/screenshot-version2.png">
</p>

## Introduction

This plugin is designed to allow you to use text sources in OBS to show a countdown timer that updates in real time where you can have multiple timers. It is designed using the OBS C/C++ API and is therefore a native plugin.

This plugin is designed to be used with `OBS version 30` and above.

## Usage

For usage instruction please refer to the [Wiki](https://www.ashmanix.com/wiki/doku.php?id=coding_projects:obs_plugin_countdown_timer).

## Build Instructions

To build this code use the included scripts that are taken from the [OBS Plugin Template](https://github.com/obsproject/obs-plugintemplate).

### GitHub Actions & CI

The scripts contained in github/scripts can be used to build and package the plugin and take care of setting up obs-studio as well as its own dependencies. A default workflow for GitHub Actions is also provided and will use these scripts.

#### Retrieving build artifacts

Each build produces installers and packages that you can use for testing and releases. These artifacts can be found on the action result page via the "Actions" tab in your GitHub repository.

#### Building a Release

Simply create and push a tag and GitHub Actions will run the pipeline in Release Mode. This mode uses the tag as its version number instead of the git ref in normal mode.
