FROM debian:bookworm-slim

# Argumente für den Benutzer (können beim Build angepasst werden, wenn nötig)
ARG USERNAME=devuser
ARG USER_UID=1000
ARG USER_GID=${USER_UID}

# Umgebungsvariablen setzen, um interaktive Abfragen während der Installation zu vermeiden
ENV DEBIAN_FRONTEND=noninteractive

# System aktualisieren und notwendige Pakete installieren
# build-essential: Enthält gcc, g++, make etc.
# gdb: Der GNU Debugger
# libncurses-dev: Entwicklungsdateien für ncurses (Header, Bibliotheken)
# git: Versionskontrolle
# sudo: Damit der Benutzer im Container bei Bedarf Root-Rechte erlangen kann
# man-db: Für den Zugriff auf Manpages (sehr nützlich)
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    gdb \
    libncurses-dev \
    git \
    sudo \
    man-db \
    ca-certificates \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

# Erstelle einen nicht-privilegierten Benutzer
# Und konfiguriere sudo so, dass dieser Benutzer ohne Passwort sudo verwenden kann
RUN groupadd --gid ${USER_GID} ${USERNAME} \
    && useradd --uid ${USER_UID} --gid ${USER_GID} -m ${USERNAME} \
    # Füge den Benutzer zur sudo-Gruppe hinzu (Debian-Weg)
    && usermod -aG sudo ${USERNAME} \
    # Alternativ: Direkter Eintrag in sudoers (benötigt visudo oder sorgfältige Behandlung)
    # && echo ${USERNAME} ALL=\(root\) NOPASSWD:ALL > /etc/sudoers.d/${USERNAME} \
    # && chmod 0440 /etc/sudoers.d/${USERNAME}
    # Die usermod-Methode ist oft einfacher, wenn die sudo-Gruppe bereits richtig konfiguriert ist.
    # Für Debian ist die Gruppe 'sudo' die richtige.
    && sed -i '/%sudo[[:space:]]/ s/ALL$/NOPASSWD:ALL/' /etc/sudoers


# Setze den erstellten Benutzer als Standardbenutzer
USER ${USERNAME}

# Setze das Arbeitsverzeichnis
WORKDIR /workspace

# Standard-Shell auf Bash setzen (optional, oft schon Standard)
SHELL ["/bin/bash", "-c"]
