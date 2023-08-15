package FTSMMC.SVPG;

import FTSMMC.models.State;
import FTSMMC.modal.formulas.Formula;

import java.util.HashSet;
import java.util.Set;

public class Vertex {
    public int owner;
    public int prio;
    public Set<Edge> edges;

    public State originState;
    public Formula originFormula;

    public Vertex() {
        this.edges = new HashSet<>();
    }

    public void addEdge(Edge e) {
        this.edges.add(e);
    }
}
