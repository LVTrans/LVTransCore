# LVTrans-

Modern LVTrans written in C++.

## Requirements

The plant should be able to:

| Done | Requirement                                                     | Designed in architecture? | Notes                                           |
| ---- | --------------------------------------------------------------- | ------------------------- | ----------------------------------------------- |
| [x]  | Load existing plant from file                                   | OK                        |                                                 |
| [x]  | Save plant to file                                              | OK                        |                                                 |
| [x]  | Step simulation                                                 | OK                        |                                                 |
| [x]  | Step simulation _N_ times                                       | OK                        |                                                 |
| [ ]  | Run simulation continuously                                     | OK                        | Maybe the client should be responsible instead? |
| [ ]  | Stop simulation                                                 | OK                        |                                                 |
| [ ]  | Pause simulation                                                | OK                        |                                                 |
| [ ]  | Resume simulation                                               | OK                        |                                                 |
| [x]  | Save current simulation state                                   | OK                        |                                                 |
| [x]  | Continue simulation from a loaded state                         | OK                        | Via constructor                                 |
| [ ]  | Read element outputs/state while the simulation is running      | OK                        | Via visitor pattern                             |
| [ ]  | Change supported runtime inputs while the simulation is running | OK                        |                                                 |
| [ ]  | Expose simulation output independently of the user interface    |                           |                                                 |
| [ ]  | Support execution faster than real time                         | OK                        |                                                 |
| [ ]  | Support multiple independent simulation instances               | OK?                       |                                                 |

## Examples

_Load plant, step, step 100 times, and save plant along with the current state._

```cpp
int main(){
    Plant plant("example_plant.yaml");

    plant.step();

    plant.run_steps(100);

    plant.save("example_plant.yaml");
}
```

_Get a list of all the elements in the plant._
_Get an element by id and its state._

```cpp
int main(){
    Plant plant("example_plant.yaml");
    // std::variant ElementState<PipeState, ValveState, PumpState> state;

    std::vector<Element> elements = plant.get_elements();
    // expecting it to be pipe
    // get_element automatically dynamic_casts to the correct type
    Pipe& pipe1 = plant.get_element("pipe_1");

    // ElementStateVisitor? It knows how to talk to the element and get its state
    ElementVisitor state_visitor = new ElementStateVisitor();
    pipe1.accept(state_visitor); // Maybe print/draw to ui?


    Valve valve1 = plant.get_element("valve_1");
    ValveState valve1_state = valve1.get_state(); // ?

    // ...
```

_Run simulation continuously until stopped._ ??

```cpp
int main(){
    Plant plant("example_plant.yaml");

    plant.run_continuous(); // infinite loop until stopped?
}
```

_Change runtime element's parameters_

```cpp
int main(){
    Plant plant("example_plant.yaml");

    // modify element checks that
    // 1. the element exists given the id
    // 2. the combination of state type, modification type and modification value is valid for that element
    plant.modify_element("valve_1", VALVE_STATE, true);
    plant.modify_element("pelton_1", PELTON_INJECTOR, NON_LINEAR);
}
```

_Add elements to a plant_

```cpp
int main(){

    Reservoir reservoir1(ReservoirParameters{....});
    Pipe pipe1(PipeParameters{....});
    Valve valve1(ValveParameters{....});


    PlantConfiguration plant_config{...};
    Plant plant(plant_config);
    // internally:
    // {
    //  std::vector<std::shared_ptr<Port>> ports;
    //  ports.push_back(new Port(this));
    //  ports.push_back(new Port(this));
    // }



    // 1. specify right and left here
    plant.add_pipe(pipe1, reservoir1, valve1);
    // internally:
    // {
    //  pipe1.connect_left(reservoir1);
    //  {
    //      ports[LEFT_IDX].connect(reservoir1);
    //  }
    //  pipe1.connect_right(valve1);
    //  {
    //      ports[RIGHT_IDX].connect(valve1);
    //  }
    // }
    // or
    plant.add_element(pipe1, reservoir1, valve1);
    plant.add_element(reservoir, nullptr, pipe1);
    // 2. specify after adding the pipe
    plant.add_element(&pipe1);
    pipe1.set_left(reservoir1); // automatically sets reservoir1's right pipe1?
    pipe1.set_right(valve1);
    // plant should now have all elements connected

```

## System Architecture (WIP)

![System Architecture](assets/system_architecture.png)
