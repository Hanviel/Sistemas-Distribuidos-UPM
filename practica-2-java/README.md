# Práctica 2: Middleware Editor/Subscriptor con Java RMI (PubSub)

Sistema de mensajería distribuido basado en el patrón **Editor/Subscriptor** (Publish/Subscribe) desarrollado en **Java RMI** (*Remote Method Invocation*), centrado en un nodo intermediario (**Broker**).

---

## 🛠️ Arquitectura y Modelo de Dominio

El sistema ofrece desacoplamiento espacial y temporal entre productores y consumidores de información mediante un Broker centralizado.

```text
 [ Editor ] --(publish)--> [ Broker (PubSubImpl) ] --(getEvent/consumeEvent)--> [ Subscriptor ]
                                  |
                   (callback: topicAdded/topicRemoved)
                                  v
                       [ SubscriberCallback ]
```

### Principales Características
* **Estructura Jerárquica de Temas:** Los temas (*topics*) se organizan como rutas POSIX (`/nivel1/nivel2/tema`).
* **Subscripción Avanzada con Patrones Glob:** Integración con la API `PathMatcher` para permitir subscripciones masivas usando comodines (`*`, `**`, `?`, `[...]`).
* **Doble Modo de Consumo:**
  1. **Editor/Subscriptor (Pull):** Los eventos publicados en un tema se distribuyen a las colas individuales de cada subscriptor registrado.
  2. **Cola de Mensajes (Message Queue):** Consumo destructivo y directo sobre el tema mediante `consumeEvent()`.
* **Notificaciones por Callback (Metaeventos):** Notificación en tiempo real a clientes interesados cuando se crean o destruyen temas en el sistema mediante llamadas RMI inversas.
* **Manejo Robusto de Excepciones:** Limpieza automática de subscriptores caídos o desconectados mediante captura defensiva de `RemoteException`.

---

## 📜 Estructura del Código y Componentes

| Paquete / Fichero | Descripción |
| :--- | :--- |
| **`common/` (`pubsub`)** | Interfaces RMI compartidas entre cliente y servidor (`PubSub`, `Subscriber`, `SubscriberCallback`, `Event`). |
| **`broker_node/` (`broker`)** | Lógica central del sistema (`PubSubImpl`, `SubscriberImpl`, `Topic`, `Server`). |
| **`client_node/` (`pubsubcln`)** | Encapsulación de cliente RMI y programas interactivos de prueba (`Client`, `Test`). |

---

## 📂 Estructura de Ficheros

```text
practica-2-java/
├── README.md               # Este documento
├── compile_all.sh          # Script para compilar todos los componentes
├── common/                 # Definiciones de interfaces RMI y clase Event
├── broker_node/
│   ├── compile.sh          # Compilación del nodo Broker
│   ├── start_rmiregistry.sh# Inicio del registro RMI
│   ├── execute_broker.sh   # Arranque del servicio Broker
│   └── src/broker/         # Implementaciones PubSubImpl, SubscriberImpl, Topic
└── client_node/
    ├── compile.sh          # Compilación de aplicaciones cliente
    ├── execute.sh          # Script ejecutor de pruebas
    └── src/
        ├── pubsubcln/      # Conexión cliente RMI (Client.java)
        └── pubsubapps/     # Clientes de prueba (Test, TestPublisher, TestSubscriber)
```

---

## 🚀 Compilación y Ejecución Paso a Paso

### 1. Compilación Global
Desde la raíz del directorio `practica-2-java`:
```bash
./compile_all.sh
```

### 2. Iniciar RMI Registry y Servidor Broker
Abre un terminal en `broker_node` y ejecuta:
```bash
cd broker_node

# Iniciar el RMI Registry en el puerto deseado (ej. 54321) en segundo plano
./start_rmiregistry.sh 54321 &

# Arrancar el servidor Broker
./execute_broker.sh 54321
```

### 3. Ejecutar la Aplicación de Prueba Interactiva
En otro terminal, inicia el cliente interactivo indicando el host y puerto del RMI Registry:
```bash
cd client_node
./execute.sh Test localhost 54321
```

### Comandos Disponibles en la Consola de Prueba
* `createTopic`: Crea un nuevo tema jerárquico.
* `initSubscriber`: Registra un nuevo subscriptor en el Broker.
* `subscribe` / `subscribeGlob`: Suscribe al cliente a temas específicos o mediante patrones.
* `publish`: Publica un mapa de claves-valores en un tema.
* `getEvent`: Extrae el siguiente evento encolado para el subscriptor.
* `consumeEvent`: Consume un evento en modo Cola de Mensajes.
