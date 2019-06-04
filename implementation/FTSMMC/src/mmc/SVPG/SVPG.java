package mmc.SVPG;

import mmc.features.FeatureDiagram;
import mmc.modal.formulas.Formula;
import mmc.models.State;

import java.util.ArrayList;
import java.util.List;

public class SVPG {
    private List<Vertex> vertices;

    public SVPG()
    {
        vertices = new ArrayList<>();
    }
    public Vertex findVertex(State originState, Formula originFormula)
    {
        for(Vertex v : vertices)
        {
            if(v.originFormula.equals(originFormula) && v.originState.sameStateNr(originState))
                return v;
        }
        return null;
    }

    public Vertex addVertex(State originState, Formula originFormula)
    {
        Vertex v = new Vertex();
        v.originState = originState;
        v.originFormula = originFormula;
        vertices.add(v);
        return v;
    }

    public void makeInfinite(){
        Vertex l0 = new Vertex();
        l0.owner = 0;
        l0.prio = 1;
        vertices.add(l0);

        Edge e0 = new Edge();
        e0.configurations = 1;
        e0.target = l0;
        l0.addEdge(e0);

        Vertex l1 = new Vertex();
        l1.owner = 0;
        l1.prio = 0;
        vertices.add(l1);

        Edge e1 = new Edge();
        e1.configurations = 1;
        e1.target = l1;
        l1.addEdge(e1);

        for(Vertex v : this.vertices)
        {
            int edgeConjunction = 0;
            for(Edge e : v.edges)
                edgeConjunction = FeatureDiagram.PrimaryFD.or(edgeConjunction, e.configurations);
            if(edgeConjunction != 1){
                Edge sinkEdge = new Edge();
                sinkEdge.target = (v.owner == 0)?l0:l1;
                sinkEdge.configurations = FeatureDiagram.PrimaryFD.not(edgeConjunction);
                v.addEdge(sinkEdge);
                sinkEdge = null;
            }
        }
    }
}
