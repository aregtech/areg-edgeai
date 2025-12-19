# Areg Edge AI Agent

An Edge AI agent service provider and consumer built on top of the
[Areg SDK](https://github.com/aregtech/areg-sdk) for distributed and event driven systems.

> [!IMPORTANT]
> This repository is under active construction and may not be fully functional yet.
> Follow the repository to receive updates as development progresses.

---

## Why Edge AI on Areg

[Areg SDK](https://github.com/aregtech/areg-sdk) is a framework with tools for building
distributed applications where communication, concurrency, and service lifecycle
are first class concepts. It is designed for systems that require predictability,
clear ownership, and strong runtime observability.

This repository demonstrates how Edge AI agents can be modeled as
Areg services. Each agent acts as a service provider or a consumer,
communicating through well defined interfaces instead of ad hoc message passing.
Areg manages discovery, message routing, threading, and fault isolation,
allowing AI logic to remain focused, testable, and reusable.

A key strength of this approach is observability.
Areg provides structured logging that captures service interactions,
message flow, and per method performance metrics.
These logs can be inspected in real time or offline using
[Lusan](https://github.com/aregtech/areg-sdk-tools),
the official UI toolset for debugging and monitoring Areg based systems.

In practice, this means Edge AI systems built on Areg are transparent,
traceable, and ready for production from the start.

---

## Planned Use Cases

The following use cases illustrate how Areg can be applied to Edge AI systems.
These cases represent the intended direction of the project.

---

### Case 1: One AI agent serving multiple clients

**This use case is part of the current implementation.**

A single AI agent receives text processing requests from multiple clients.
Clients may appear or disappear on the local network at any time.
When the agent is online, clients can submit requests and receive responses.
Requests are queued on the agent side, and each response is delivered
to the correct client without mixing results.

**Key features to demonstrate:**
1. No startup order is required. The AI Service Provider and the Service Consumers can join or leave the network at any moment without destabilizing the system.
2. Requests are queued on the AI Service Provider side. Each reply is delivered to the correct client.
3. Automatic service discovery. When the AI Service Provider becomes available on the network, all Service Consumers receive a service available notification and can start communicating.

---

### Case 2: Multiple AI agents managed by a central service

A central service provider receives requests from many clients.
For each request, the provider starts a dedicated thread or process
hosting an AI engine instance.
Processing runs in parallel and results are returned to the provider,
which forwards them to the originating clients.

**Key features to demonstrate:**
1. No startup order is required. Service Provider and Service Consumers join or leave the network freely.
2. Requests are not queued. Each request immediately triggers a new thread or process with its own AI engine instance. When processing is complete, the central provider sends the response to the requester.
3. Automatic service discovery with the same behavior as in Case 1.
4. The system can instantiate many independent AI engine instances.

---

### Case 3: Dynamic AI skill marketplace

Each small Edge AI agent service represents a specific skill:
- One skill rewrites text.
- Another compresses it.
- Another expands it.
- Another evaluates or rates it.

Clients publish a goal such as generating a short description
with a score above a given threshold.
Any available skill that matches the requirement may claim the task.
The system behaves like a decentralized marketplace of capabilities.

**Key features to demonstrate:**
1. Auto-discovery of services with specific capabilities.
2. Decentralized task assignment with no single point of failure.
3. Multi-role behavior: any node may be both consumer and provider.
4. Runtime expansion: new skills can appear and work instantly.

---

### Case 4: Scenario simulation with AI agents as characters

A virtual story world is created where each agent represents a character
with a defined personality.
A central controller introduces scenarios such as investigating a mystery,
planning a project, or debating a topic.
Agents exchange messages according to predefined rules and react to each
other’s outputs.

**Key features to demonstrate:**
1. Many independent agents interacting in a shared context.
2. State synchronization through AREG events.
3. Multi-room or multi-scenario orchestration.
4. Dynamic participation: new characters can join or leave at any time.

---

### Case 5: Multi stage distributed evaluator

A quality evaluation pipeline is composed of multiple evaluator agents:
- One evaluates clarity.
- Another evaluates style.
- Another checks consistency.
- Another evaluates creativity.

A coordinator aggregates the individual scores and produces a final rating.

**Key features to demonstrate:**
1. Many small services contributing to a single final result.
2. Parallel evaluations.
3. Redundant evaluators: start three clarity evaluators and one is chosen automatically.
4. Service discovery and role specialization.

---

### Case 6: Hierarchical and cross connected agents

A central register application monitors and coordinates multiple agent applications.
Each agent hosts its own AI engine and has a unique name.
Agents and the register may run on the same machine or on different machines.

The register can create discussion rooms by selecting multiple agents
and providing a topic.
Each agent generates an initial response.
All participants receive the initial texts and continue a shared discussion.
Multiple discussion rooms may exist at the same time,
and agents may participate in more than one room concurrently.

**Key features to demonstrate:**
1. Many AI agent services can run in the network at the same time.
2. Every agent can act as both service provider and service consumer.
3. Mesh style communication. Even if the register application shuts down, existing discussion rooms continue to function. Texts do not leak between rooms and each topic remains isolated.
4. Fault tolerance. Any agent or the register can disconnect and reconnect without interrupting ongoing communication among other agents.
5. Horizontal and vertical use. Agents can consume each other’s services or provide services to each other without restrictions.


> [!NOTE]
> **These use cases are planned and may change over time.**
> **Some may be combined, refined, or replaced as development progresses.**
> **Feedback and feature requests are welcome.**

---

## Call to Action

This project is at an early stage and actively looking for contributors.

If you are interested in Edge AI, distributed systems, service oriented architecture,
or C and C++ development, consider joining the effort.
Contributions are welcome in code, testing, documentation, integration,
or design discussions.

---

## License

This project is licensed under the MIT License.
See the [LICENSE](LICENSE) file for details.
