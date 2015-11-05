# How to contribute to the CMaNGOS project

There are many different ways to contribute to CMaNGOS to continue improving it!

These guidelines shall help you by providing suggested ways of how you can help.

As CMaNGOS and its friendly projects are a huge and widespread way of working,
you can dig in at any place you want to!

*Work for any part is appreciated!*

See our linklist at [CMaNGOS Links](http://cmangos.net/thread-26.html) for other places.

This guide however is only for contributing to the CMaNGOS core part -
which is hosted at https://github.com/cmangos

## Getting Started

* Make sure you have a [Forum account](http://cmangos.net)
* Make sure you have a [GitHub account](https://github.com/signup/free)
* Watch the CMaNGOS repository on GitHub (This way you will get informed about changes by GitHub)
* Optionally: Fork the CMaNGOS repository on GitHub in order to be able to publish own changes and create pull requests with them (see below for details)

You can help us developing the core by (at least!)
* Giving Feedback on Bugs
* Testing features in development
* Writing own patches

# Bug-Reports

If you spot a bug, please report it by opening an issue on our centralized GitHub issue tracking repository: [cmangos/issues](https://github.com/cmangos/issues/issues).
Before you do so, please consider the following points:

* The bug happens on a clean core
* You can reproduce the bug
* The bug is not already reported. If it is you are however very welcome to add a "for me too" comment to the existing issue

When you decide to create a new bug report, please make sure that this report includes:

* Revision on which you encountered this bug (if known, also the revision or timeframe where it first appeared) - you can also post the link to the commit on the upstream repository
* Revisions of additional parts of the CMaNGOS familiy (SD2 version, database name and version, and such)
* A detailed step-by-step report how this bug can be reproduced
* A detailed explanation of what DOES happen
* A detailed explanation of what SHOULD happen  - if possible also add a source why you think this should happen
* When reporting spell-related bugs, please use spell-IDs in your description, and try to use .cast commands to reproduce your bug. This makes reproducing much easier!
* When reporting crashes, please include a crash-log (as detailed as possible - we like GDB logs from debug builds!)
* Only report one bug per issue/thread

# Testing features in development
## Importing pull-requests
Merging pull-requests into your branch for testing can be done by the GUIs provided by GitHub.
For people not wanting to use them, we have a "MergeHelper.sh" script shipped in contrib/

You can - also on Windows within a Git Bash - start the script by using
```
contrib/MergeHelper.sh -h
```

This script operates in two modes:
* Pass the pull-request number you want to merge as parameter, i.e.
```
contrib/MergeHelper.sh 37
```
which will simply merge pull-request 37 from your origin repository

* You can use the script with a (created) config file to automatically fetch+merge repositories,
  branches and pull-requests

  This should be especially reasonable, if you yourself use topic branches like described in the next section!

Please be sure to feedback your test-results through proper channels -- this means commenting on the pull-requests, commenting on issues and commits on GitHub.


## Importing features from third-party repos

Sometimes you might want to import commits from a third-party repository.

For this you need to setup a remote pointing to this repository
```
git remote add <Name> git://github.com/USER/REPO.git
```
and then git fetch it and merge the branches you are interested in.

Also note that you can easily use the MergeHelper script mentioned above to automate fetch+merge after initial setup!

# Contributing patches

## Topic branches
We suggest to use so-called topic-branches for developing some contribution.
The idea is to bundle all required commits for a project into one topic branch.
This topic branch is expected to be published and shared very fast, so that people can comment on your work as soon as possible

To easily create a topic branch we have added a small helper script, that will guide you interactively through
the process of setting up a topic branch (even if this is basic git-use).

You can start the script by using
```
contrib/CreateTopicBranch.sh
```

While developing you should try to keep these points in mind:
* *Commit often, commit early*
* *Write good commit messages to tell people what you are trying to do in your commit*
  This must include some test instructions, link to bug-report and similar

Also we expect you to keep our [Coding Style](https://github.com/cmangos/mangos-wotlk/wiki/Coding-Standards)
This code style is automatically enforced by the helper scripts in contrib/cleanupTools (which require AStyle to be installed)

## Deployment

You can publish and share your contribution projects via forum or by pull-requests

When you have created your topic-branch and pushed it to your local repository, you can easily create a pull-request to our repo from this!
Please always create the pull request on the repository containing the CMaNGOS code you were working on.

Here is the [GitHub Information on Pull-Requests](https://help.github.com/articles/using-pull-requests)

This has these advantages:
* Your code will be read and commented on by many people
* Your code will be fetched by people willing to help testing
* Once you have opened a pull-request for a branch, every change of the branch (like a new commit) will automatically be added to the pull-request.
  So you won't have any additional overhead in maintaining the pull-request :)
* Travis CI will automatically test the build of your pull request and report the status just as it does for our commits

If it is only a minor change and you do not want to create a topic-branch and a pull request (though you should, it's fast and easy to do!),
you can also open a normal issue and paste the diff in there (but please use the Markdown tags to mark it as a code!). Rule of thumb is that
more than 20 lines should most definitely be posted as a pull request instead of an issue with a simple diff.

# Additional Resources

* [General GitHub documentation](http://help.github.com/)
* [GitHub pull request documentation](http://help.github.com/send-pull-requests/)
* #cmangos IRC channel on irc.rizon.net
