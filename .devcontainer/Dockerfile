FROM debian:bookworm-slim

# Arguments for user creation. These can be overridden at build time if needed.
ARG USERNAME=devuser
ARG USER_UID=1000
ARG USER_GID=${USER_UID}

# Set DEBIAN_FRONTEND to noninteractive to prevent interactive prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive

# Update package lists and install essential packages for C development and general use
# --no-install-recommends reduces the number of installed packages
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    # build-essential: Includes C/C++ compilers (gcc, g++), make, etc.
    gdb \
    # gdb: The GNU Debugger, for debugging C programs.
    libncurses-dev \
    # libncurses-dev: Development files (headers and libraries) for ncurses UI.
    git \
    # git: Version control system.
    sudo \
    # sudo: Allows the created user to run commands with root privileges.
    man-db \
    # man-db: For accessing manual pages (e.g., `man printf`).
    ca-certificates \
    # ca-certificates: Provides SSL certificates for secure connections (e.g., for git clone https).
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/* # Clean up apt cache to reduce image size.

# Create a non-root user with specified UID/GID and add to sudoers with NOPASSWD
RUN groupadd --gid ${USER_GID} ${USERNAME} \
    && useradd --uid ${USER_UID} --gid ${USER_GID} -m -s /bin/bash ${USERNAME} \
    # Add user-specific sudo rule for passwordless sudo.
    # This is generally preferred for dev containers over modifying /etc/sudoers directly.
    && echo ${USERNAME} ALL=\(root\) NOPASSWD:ALL > /etc/sudoers.d/${USERNAME} \
    && chmod 0440 /etc/sudoers.d/${USERNAME}

# Switch to the created non-root user
USER ${USERNAME}

# Set the default working directory for subsequent commands and for the container runtime
WORKDIR /workspace

# Set the default shell for RUN, CMD, ENTRYPOINT instructions.
# This is often /bin/sh by default, which might be dash on Debian.
# Explicitly setting to bash ensures bash-specific syntax works if used in RUN commands.
SHELL ["/bin/bash", "-c"]