# Práctica 1: Red P2P en Anillo para Compartir Contenidos (RING)

Aplicación distribuida en **C** que implementa una red Peer-to-Peer (**P2P**) estructurada con topología en anillo para la búsqueda y transferencia eficiente de ficheros en un entorno de red heterogéneo.

---

## 🛠️ Arquitectura y Diseño Técnico

A diferencia de protocolos como Chord que emplean un esquema de *fingers* con complejidad de búsqueda $O(\log n)$, este sistema utiliza una **búsqueda lineal acotada** con complejidad $O(n)$, priorizando la simplicidad y el control explícito del flujo de red.

### Principales Características
* **Modelo Simétrico P2P:** Cada nodo ejecuta el mismo binario (`ring`) y actúa simultáneamente como:
  * **Cliente:** Procesa comandos de interfaz local y encamina peticiones por el anillo.
  * **Servidor Concurrente:** Escucha conexiones entrantes en un puerto dinámico y lanza un hilo dedicado (`pthread`) por cada petición entrante.
* **Mantenimiento del Anillo (Bootstrap):** Los nodos se incorporan notificando a un "nodo de contacto" existente, reconfigurando automáticamente las referencias de sucesores.
* **Técnicas de Optimización Zero-Copy:**
  * **Envío con `sendfile`:** Transfiere datos desde el sistema de archivos al socket directamente en espacio de núcleo, evitando copias a memoria de usuario.
  * **Recepción con `mmap`:** Asigna el fichero de destino directamente en memoria antes de la recepción por el socket.
  * **Control de Fragmentación:** Uso de `writev` o la bandera `MSG_MORE` en `send()` al transmitir cabeceras y parámetros de longitud variable.

---

## 📜 Fases de Desarrollo e Interfaz (API)

El sistema se estructura en 5 fases incrementales basadas en las funciones exportadas por `ring_cln.h`:

| Fase | Operación API | Descripción |
| :--- | :--- | :--- |
| **Fase 1** | `ring_init`, `ring_remote_pid` | Inicialización de hilos servidores y verificación de PIDs remotos. |
| **Fase 2** | `ring_successor`, `ring_remote_successor_successor` | Gestión de altas en el anillo y propagación de consultas a sucesores remotos. |
| **Fase 3** | `ring_download` | Descarga directa nodo a nodo usando transferencias de alto rendimiento (`sendfile`/`mmap`). |
| **Fase 4** | `ring_lookup` | Algoritmo de búsqueda distribuida acotado por un límite de saltos (*hops*). |
| **Fase 5** | `ring_get_file` | Integración de localización automática (`lookup`) y posterior descarga directa (`download`). |

---

## 📂 Estructura de Ficheros

```text
practica-1-c/
├── README.md               # Este documento
├── ejemplos/               # Ejemplos base de sockets, threads, mmap y sendfile
└── src/
    ├── Makefile            # Script de compilación GCC
    ├── main.c              # Interfaz de usuario interactiva y parseo de argumentos
    ├── ring_cln.c          # Lógica del cliente P2P y envío de mensajes
    ├── ring_srv.c          # Servidor multihilo y despacho de operaciones
    ├── common.c            # Utilidades de sockets TCP y gestión de threads
    └── include/
        ├── ring.h          # Interfaz principal de la aplicación
        └── common.h        # Cabeceras y estructuras auxiliares
```

---

## 🚀 Compilación y Ejecución

### 1. Compilar el Proyecto
```bash
cd src
make
```

### 2. Crear el Primer Nodo (Bootstrap de la Red)
Inicia la red P2P especificando el directorio de archivos que compartirá el nodo:
```bash
mkdir dir1
./ring dir1
```

### 3. Incorporar Nodos Adicionales
Para unir un nuevo nodo al anillo, se indica su directorio compartido más la IP y puerto de cualquier nodo activo:
```bash
# En otro terminal o equipo
mkdir dir2
./ring dir2 127.0.0.1 <PUERTO_NODO_1>
```

### 4. Comandos de la Interfaz Interactiva
Al arrancar, el programa muestra un menú de opciones textuales:
* **`I`**: Muestra la información del nodo local (IP, Puerto, PID).
* **`S`**: Consulta el nodo sucesor directo en el anillo.
* **`L`**: Realiza un *lookup* de un fichero indicando el máximo de saltos.
* **`G`**: Busca y descarga automáticamente un fichero localizado en la red P2P.
