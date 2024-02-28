**HPC-NOW, start your HPC journey in the cloud <u>now</u>, with <u>n</u>o <u>o</u>peration <u>w</u>orkload!**

*<u>A full-stack HPC solution in the cloud, for the HPC community.</u>*

**Your contributions bring great value to this project.**

- [1. Preface](#1-preface)
- [2. Roadmap](#2-roadmap)
- [3. Issues](#3-issues)
- [4. Discussions](#4-discussions)
- [5. Pull Rquests](#5-pull-rquests)
- [6. Dos and Don'ts](#6-dos-and-donts)

# 1. Preface

The first and easiest way to contribute to an open-source project, is to let more people know it. 

Therefore, we kindly request a **github star**, **watch**, and **fork** from you. 

With your contribution, this project will become more and more valuable to you, and the whole open-source community.

The [README.md](./README.md) in this repo contains the background of this project, an brief intro to the core components, a guideline to build/install/run, and a brief usage instruction. We strongly recommend you to read it first.

An efficient way to learn a product is to use it. Therefore, we suggest you to build, install, and run this software. 
  
This project covers several different technologies, including:

- **Cloud infrastructure-as-code**: code and manage the cloud resources
- **Multi-cloud management**: the core products of cloud computing across different cloud service providers
- **HPC/AI workload scheduling**: we use SLURM as the core scheduler
- **GNU/Linux, Shell scripts, and automation**: we automate not only the cloud resources, but also everything else
- **The C programming**: this project uses C to develop Command Line Interface (CLI), and other core components
- **HPC/AI cluster management**: including data transfer and management, multi-user management, HPC application management, and HPC/AI job management
- **HPC/AI software management**: automatically manage the software (sources, prebuilt packages, etc.)
- ...

We aim to building an open and free(libre) cloud platform to the HPC/AI community. This is why your contributions are very important.

# 2. Roadmap

There are several key work to be done. Please check the list below:

- **Continuous Enhancement**: make the **hpcopr** CLI more functional and robust.
- **GPU for AI**: the HPC-NOW started from High-Performance Computing, therefore, GPU cluster with SLURM was not included. We aims to managing GPU clusters for AI workloads in the near future
- **Graphical User Interface (GUI)**: GUI is always much more user-friendly compare to CLIs. We need a cross-platform GUI for this project.
- **Web-based UI (WUI)**: in order to make the **HPC Cloud** real and accessible for everyone, a web-based UI is essential.
- **APPlication MANager (known as 'appman')**: a package manager for building/installing/removing HPC/AI applications.
- **ARM64 Compatibility**: make HPC-NOW work on mainstream ARM devices.

This list is not detailed or completed, but it shows how this project will evolve: from a CLI (C/S architecture) to a cloud platform (B/S architecture).

Please feel free to propose your ideas.

# 3. Issues

Like any software product, bugs and problems may probably occur. It is our job to solve the problems and make the software better. 

Therefore, please submit issues to this repository. In order to debug and communicate efficiently, please follow the guidelines below:

- **Running environment**: OS version and HPC-NOW version code
- **Problem description**: the more detailed, the better. Feel free to use screenshots (**please take care of your privacy when uploading screenshots**)
- **Expected behaviors**: the behaviors that are expected and accepted

Other than bugs and problems, you can also submit feature requests or other relavent ideas. Please follow the guidelines below:

- **Description**: Please describe your ideas/feature requests in detail. Pictures, architectures, or drawings are welcomed.
- **References**: If there are good practices, please also provide them.

# 4. Discussions

We believe that **discussions generate great ideas**. Therefore, please feel free to interact with others in the discussion pages published in this repo.

# 5. Pull Rquests

Code contributions to HPC-NOW are highly welcomed and encouraged!

HPC-NOW can be developed locally on your devices with **Microsoft Windows**, **GNU/Linux**, or **macOS(Darwin)**.

Please follow the general workflow to develop:

- Fork this repo to your github account and then clone to your local device
- Develop your code and test your build
- Once done, you can submit a Pull Request (PR) to the HPC-NOW repository.

Please provide the key information in your PR:

- **Description of the Change**: [ Detailed, descriptive, well-structured, and well-organized contents. ]
- **Benefits**: [ What are the effects/benefits will be realized by the changed code? ]
- **Potential Risks**: [ Will there be potential side-effects? If yes, please describe them. ]
- **Verification**: [ Steps to verify the that your change will realize the benefits claimed. ]
- **Related-Issues**: [ Provide the issue number. If your change closes an open issue, use *Closes Issue #xxxxx* ]

Looking forward to your contributions!

Once the PR gets reviewed, the code change will be merged by the workflow.

# 6. Dos and Don'ts

- **Focus**: Please focus on HPC-NOW itself. Unreleated issues or topics are not supposed to be here.
- **Inclusive**: Keep offensive interactions or attitudes out of this community.
- **Respect**: Value our own efforts, and respect others' participation.