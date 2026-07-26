# Práctica 2: Sistema PubSub con Java RMI

## 📌 Resumen del Enunciado
Desarrollo de un sistema Editor/Subscriptor usando Java RMI. El sistema se basa en un nodo central (Broker) y clientes (Editores y Subscriptores). Soporta temas jerárquicos (ej. `/nivel1/tema1`), subscripciones masivas mediante patrones glob (`*`, `?`), y notificaciones automáticas (callbacks) cuando se crean o destruyen temas. Ofrece dos modos de consumo: el tradicional esquema *Pull* (Editor/Subscriptor) y un modo destructivo tipo Cola de Mensajes.

## 🚀 Cómo Ejecutar

```bash
# 1. Compilar todo el proyecto
./compile_all.sh

# 2. Iniciar el RMI Registry y el Broker
cd broker_node
./start_rmiregistry.sh 54321 &
./execute_broker.sh 54321

# 3. Ejecutar pruebas con clientes
cd ../client_node
./execute.sh Test localhost 54321
