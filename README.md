# Prácticas de Sistemas Distribuidos (SD)

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
```

---

## 🚀 Práctica 1: Aplicación P2P RING (C / POSIX Sockets)

Desarrollo de una red **Peer-to-Peer (P2P)** estructurada en topología de anillo para la localización y transferencia de ficheros compartidos en un entorno heterogéneo.

### 🔑 Características Clave
* **Arquitectura Simétrica:** Cada nodo actúa como cliente (atención a usuario/encaminamiento) y servidor concurrente (basado en hilos dinámicos `pthread`).
* **Optimización Zero-Copy:** 
  * Transferencia de archivos mediante `sendfile()` en el emisor para evitar copias de memoria a espacio de usuario.
  * Mapeo en memoria mediante `mmap()` y `recv()` en la recepción.
  * Prevención de fragmentación de paquetes con `writev()` / `MSG_MORE`.
* **Búsqueda Lineal Bounded ($O(N)$):** Algoritmo distribuido para localización de recursos limitando el número de saltos (`hops`).

### 🧪 Entorno de Desarrollo y Ejecución
* **Entorno de Compilación:** GCC (GNU Compiler Collection) con soporte para POSIX Threads (`-lpthread`).
* **Plataforma Soportada:** Linux (Probado en el clúster de la universidad `triqui.fi.upm.es`, contenedores Docker `ubuntu:24.04` o máquinas virtuales Linux).
* **Compilación y Ejecución Básica:**
  ```bash
  cd practica-1-c/src
  make
  ./ring <directorio_compartido> [ip_nodo_contacto puerto_nodo_contacto]
  ```

---

## 📡 Práctica 2: Sistema PubSub (Java RMI)

Desarrollo de un middleware **Editor/Subscriptor (Pub/Sub)** con arquitectura centralizada en un **Broker**, ofreciendo desacoplamiento espacial y temporal mediante Invocación de Métodos Remotos (RMI).

### 🔑 Características Clave
* **Jerarquía de Temas y Patrones Glob:** Clasificación de temas mediante rutas tipo POSIX (`/nivel1/nivel2/tema`) con soporte de subscripciones múltiples por patrones glob (`*`, `**`, `?`, `[...]`) usando `PathMatcher`.
* **Doble Modelo de Interacción:**
  * **Editor/Subscriptor (Pull):** Los eventos se encolan por subscriptor y se recogen mediante *polling*.
  * **Cola de Mensajes (Message Queue):** Consumo destructivo de eventos directamente del tema (`consumeEvent`).
* **Notificaciones Callback:** Invocación remota hacia el cliente para notificar metaeventos de alta/baja de temas en tiempo real.
* **Tolerancia a Fallos:** Manejo defensivo de excepciones (`NoSuchObjectException`, desconexión de subscriptores caídos).

### ☕ Entorno de Desarrollo y Ejecución
* **Entorno de Ejecución:** Java JDK 11 o superior (incluye `rmiregistry` y utilidades del JDK).
* **Plataforma Soportada:** Multiplataforma / Linux (Probado en `triqui.fi.upm.es` o entornos locales con scripts `.sh`).
* **Compilación y Ejecución Básica:**
  ```bash
  cd practica-2-java
  ./compile_all.sh
  
  # 1. Iniciar Broker y RMI Registry
  cd broker_node
  ./start_rmiregistry.sh 54321 &
  ./execute_broker.sh 54321

  # 2. Iniciar Cliente / Pruebas
  cd ../client_node
  ./execute.sh Test localhost 54321
  ```
