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
* Fork the CMaNGOS repository on Github (This way you will get informed about changes by github)

You can help us developing the core by (at least!)
* Giving Feedback on Bugs
* Testing features in development
* Writing own patches

# Bug-Reports

If you spot a bug, you can either report it on our forums (cmangos.net) or by opening an issue in one of our github repositories.
Before you do so, please consider the following points:

* The bug happens on a clean core
* You can reproduce the bug
* The bug is not already reported. If it is you are however very welcome to add a "for me too" comment to the existing issue

When you decide to create a new bug report, please make sure that this report includes:

* On which revision the bug happens
* Revisions of additional parts of the CMaNGOS familiy (SD2-version, Database Name and Version, and such)
* What you need to do do trigger the bug, be as detailed as possible.
  Please use spell-ids in your description, and try to use .cast commands to reproduce your bug. This makes reproducing much easier!
* What IS happening WHEN doing your actions
* What SHOULD happen when doing your actions - if possible also add a source why you think this should happen

* Please report only one bug per issue/thread!

# Testing features in development
## Importing pull-requests
Merging pull-requests into your branch for testing can be done by the GUIs provided by Github.
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

Please be sure to feedback your test-results through proper channels -- this means commenting on the pull-requests, commenting on issues and commits on github.


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

### Via Forum
Using the our forum for sharing patches is perfectly fine, but for complicated features we strongly suggest to use topic branches and pull-requests
If you want to use the forum, please consider _also_ [Forum Patch submission Guidelines](http://cmangos.net/thread-42.html)

### Via Pull-Requests on Github
When you have created your topic-branch and pushed it to your local repository, you can easily create a pull-request to our repo from this!

Here is the [Github Information on Pull-Requests](https://help.github.com/articles/using-pull-requests)

This has these advantages:
* Your code will be read and commented on by many people
* Your code will be fetched by people willing to help testing
* Once you have opened a pull-request for a branch, every change of the branch (like a new commit) will automatically be added to the pull-request.
  So you won't have any additional overhead in maintaining the pull-request :)

# Additional Resources

* [General GitHub documentation](http://help.github.com/)
* [GitHub pull request documentation](http://help.github.com/send-pull-requests/)
* #cmangos IRC channel on irc.rizon.net
