# Prácticas de Sistemas Distribuidos

Repositorio que reúne las prácticas de laboratorio de la asignatura **Sistemas Distribuidos** (Grado en Ingeniería Informática, UPM). Contiene dos proyectos que abordan paradigmas fundamentales de la computación distribuida: **Sistemas P2P en C** y **Sistemas de Mensajería Pub/Sub en Java RMI**.

---

## 🛠️ Resumen de Proyectos

| Práctica | Nombre | Lenguaje / Tecnología | Paradigma / Arquitectura |
| :--- | :--- | :--- | :--- |
| **Práctica 1** | [RING - P2P Content Sharing](./practica-1-c/) | C, Sockets TCP, POSIX Threads | Red P2P Estructurada (Topología en Anillo) |
| **Práctica 2** | [PubSub - Editor/Subscriptor](./practica-2-java/) | Java, Java RMI, PathMatcher | Sistema Editor/Subscriptor Basado en Broker |

---

## 📂 Estructura del Repositorio

```text
.
├── .gitignore               # Exclusión de binarios (.o, .class, ejecutables, etc.)
├── README.md                # Visión general del repositorio
├── practica-1-c/            # Práctica 1: Aplicación P2P RING (C)
│   ├── README.md            # Enunciado resumido y manual de uso de P1
│   ├── Makefile             # Scripts de compilación
│   └── src/                 # Código fuente (.c y .h)
└── practica-2-java/         # Práctica 2: Sistema PubSub (Java RMI)
    ├── README.md            # Enunciado resumido y manual de uso de P2
    ├── compile_all.sh       # Script de compilación global
    ├── broker_node/         # Servidor Broker y RMI Registry
    ├── client_node/         # Clientes y aplicaciones de prueba
    └── common/              # Interfases RMI y clases compartidas
