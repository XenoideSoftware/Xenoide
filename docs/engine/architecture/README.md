# XE Engine — Architecture Models

C4 architecture models for the XE engine, authored in [Structurizr DSL](https://docs.structurizr.com/dsl) and rendered via [Structurizr Lite](https://docs.structurizr.com/lite).

## Viewing the model

```bash
cd docs/architecture
docker compose up -d
```

Open [http://localhost:8080](http://localhost:8080) in your browser.

Stop:

```bash
docker compose down
```

## Editing

Edit any `.dsl` file and refresh the browser — Structurizr Lite picks up changes automatically.

The workspace is split across files using `!include`:

```
workspace.dsl           Root workspace (includes model/ and views.dsl)
views.dsl               All views and styles
model/
  people.dsl            Personas (Application Developer, Content Author)
  systems.dsl           External systems (GPU driver, Console SDK, OS, package registry)
  containers.dsl        Containers inside XE Renderer
  components-device.dsl Components inside the Dispatch container
  components-backends.dsl  Components inside the Backends container
  components-asset.dsl  Components inside the Asset Compiler container
  relationships.dsl     All relationships between elements
```

## Adding a new subsystem

1. Create a new `model/components-<name>.dsl` with component definitions inside the relevant container.
2. Add an `!include model/components-<name>.dsl` line in `workspace.dsl`.
3. Add a `component <container>` view block in `views.dsl`.
4. Refresh the browser.

## Written spec

The authoritative written specification lives in [`../rendering-subsystem-spec.md`](../rendering-subsystem-spec.md) with per-module details in [`../rendering/`](../rendering/).
