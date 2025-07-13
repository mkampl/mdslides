# Live Shell Command Demo

**Eine interaktive Präsentation mit echten System-Commands**

Willkommen zur Demo des erweiterten ncurses Slide-Presenters!

---

## Agenda

- System-Informationen live anzeigen
- Dateisystem erkunden
- Git-Repository-Status
- Performance-Monitoring
- Netzwerk-Diagnostics

---

## Aktuelle Dateien

Schauen wir uns an, was sich im aktuellen Verzeichnis befindet:

```$ls -la
```

**Hinweis:** Drücke ENTER um den Command auszuführen!

---

## System-Zeit & Uptime

### Aktuelle Zeit:
```$date +"%Y-%m-%d %H:%M:%S"
```

### System-Uptime:
```$uptime
```

---

## Speicher-Informationen

### Festplattenspeicher:
```$df -h
```

### RAM-Verbrauch:
```$free -h
```

---

## Prozess-Monitoring

### Top 10 Prozesse:
```$ps aux | head -10
```

### Aktuelle Benutzer:
```$who
```

---

## Netzwerk-Status

### Netzwerk-Interfaces:
```$ip addr show | grep -E "inet |^[0-9]"
```

### Aktive Verbindungen:
```$netstat -tuln | head -10
```

---

## Git Repository Status

### Git-Status (falls in Git-Repo):
```$git status --short
```

### Letzter Commit:
```$git log --oneline -1
```

### Branch-Info:
```$git branch --show-current
```

---

## Development Environment

### Node.js Version:
```$node --version 2>/dev/null || echo "Node.js nicht installiert"
```

### Python Version:
```$python3 --version 2>/dev/null || echo "Python3 nicht installiert"
```

### GCC Compiler:
```$gcc --version | head -1
```

---

## Package Management

### NPM Packages (falls vorhanden):
```$npm list --depth=0 2>/dev/null | head -5 || echo "Kein npm project"
```

### System Packages (Ubuntu/Debian):
```$dpkg -l | grep -E "^ii" | wc -l
```

---

## System-Informationen

### CPU-Info:
```$lscpu | grep -E "Model name|CPU\(s\):"
```

### Memory-Info:
```$cat /proc/meminfo | grep -E "MemTotal|MemAvailable"
```

### Kernel-Version:
```$uname -r
```

---

## Performance-Tests

### Einfacher CPU-Test (5 Sekunden):
```$timeout 5s yes > /dev/null; echo "CPU-Test abgeschlossen"
```

### Disk-Speed-Test:
```$time dd if=/dev/zero of=/tmp/test bs=1M count=100 2>&1 | tail -2
```

**Achtung:** Cleanup nach dem Test:
```$rm -f /tmp/test
```

---

## Docker & Container (falls installiert)

### Docker-Version:
```$docker --version 2>/dev/null || echo "Docker nicht installiert"
```

### Laufende Container:
```$docker ps 2>/dev/null || echo "Docker daemon nicht erreichbar"
```

---

## Logs & Debugging

### Letzte System-Logs:
```$journalctl --no-pager -n 3 2>/dev/null || tail -3 /var/log/syslog 2>/dev/null || echo "Keine Log-Berechtigung"
```

### Disk-Usage der größten Verzeichnisse:
```$du -h --max-depth=1 . | sort -hr | head -5
```

---

## Custom Commands

### Erstelle eine temporäre Datei:
```$echo "Hello from $(whoami) at $(date)" > /tmp/demo.txt
```

### Zeige den Inhalt:
```$cat /tmp/demo.txt
```

### Cleanup:
```$rm /tmp/demo.txt && echo "Temporäre Datei gelöscht"
```

---

## Demo Ende

**Vielen Dank für die Aufmerksamkeit!**

### Zusammenfassung der Features:
- Live Shell-Command Ausführung
- Echtzeit-System-Informationen  
- Interaktive Demos
- Sichere Command-Bestätigung

### Nächste Schritte:
```$echo "Viel Spaß beim Experimentieren mit dem Slide-Presenter!"
```

---

## Code-Beispiel ohne Shell

Normale Code-Blöcke funktionieren weiterhin:

```cpp
#include <iostream>

int main() {
    std::cout << "Hello World!" << std::endl;
    return 0;
}
```

```bash
# Kompilierung (als normaler Code-Block)
g++ -o hello hello.cpp
./hello
```

**Der Unterschied:** ```$command``` wird ausgeführt, ```language``` wird nur angezeigt!# 🚀 Live Shell Command Demo
