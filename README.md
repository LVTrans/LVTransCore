# LVTrans-

Modern LVTrans written in C++.

## Requirements

The plant should be able to:

- [ ] Load existing plant from file
- [ ] Save plant to file
- [ ] step simulation
- [ ] Step simulation _N_ times
- [ ] Run simulation continuously
- [ ] Stop simulation
- [ ] Pause simulation
- [ ] Resume simulation

---

- [ ] Save current simulation state
- [ ] Continue simulation from a loaded state

---

- [ ] Read element outputs/state while the simulation is running
- [ ] Change supported runtime inputs while the simulation is running
- [ ] Accept runtime input independently of the user interface
- [ ] Expose simulation output independently of the user interface

---

- [ ] Support execution faster than real time
- [ ] Support multiple independent simulation instances

## Examples

```cpp
int main(){
    Plant plant("example_plant.yaml");

    plant.step();

    plant.run_steps(100);

    plant.save("example_plant.yaml");
}
```
