# Práctica 1: Aplicación P2P RING

## 📌 Resumen del Enunciado
Desarrollo de una aplicación P2P para compartir contenidos mediante una topología en anillo. A diferencia de Chord, utiliza un mecanismo de búsqueda lineal ($O(N)$). Cada nodo actúa simultáneamente como cliente y como servidor concurrente (usando un hilo por conexión). Se exige optimización de rendimiento (Zero-Copy) usando `sendfile`, `mmap`, y `writev`/`MSG_MORE` para evitar duplicación de memoria al transferir archivos.

## 🚀 Cómo Ejecutar

```bash
# 1. Compilar el proyecto
cd src
make

# 2. Iniciar el primer nodo (crea la red)
./ring <directorio_compartido>

# 3. Conectar un nuevo nodo a una red existente
./ring <directorio_compartido> <ip_nodo_contacto> <puerto_nodo_contacto>

