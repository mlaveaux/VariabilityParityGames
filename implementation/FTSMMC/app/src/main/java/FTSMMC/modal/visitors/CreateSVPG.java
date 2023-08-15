package FTSMMC.modal.visitors;

import FTSMMC.SVPG.Edge;
import FTSMMC.SVPG.SVPG;
import FTSMMC.SVPG.Vertex;
import FTSMMC.features.FeatureDiagram;
import FTSMMC.modal.formulas.*;
import FTSMMC.models.Label;
import FTSMMC.models.Lts;
import FTSMMC.models.State;

import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class CreateSVPG extends RecursiveVisitor {
    private SVPG sVPG;
    private Lts fts;
    private State currentState;
    private Map<RecursionVariable, FixedPointFormula> unfoldingMapping;

    public CreateSVPG(SVPG sVPG, Lts fts, State currentState,
            Map<RecursionVariable, FixedPointFormula> unfoldingMapping) {
        this.sVPG = sVPG;
        this.fts = fts;
        this.currentState = currentState;
        if (unfoldingMapping == null) {
            this.unfoldingMapping = new HashMap<>();
        } else {
            this.unfoldingMapping = unfoldingMapping;
        }
    }

    @Override
    public void visit(BoxFormula formula) {
        Vertex v = createVertexIfItDoesntExist(formula);
        if (v == null)
            return;
        v.owner = 1;
        v.prio = 0;

        for (Label l : currentState.getTransitionLabels()) {
            List<String> actions = Arrays.asList(formula.getAction().getText().split("\\|\\|"));
            if (!actions.contains(l.getText()))
                continue;
            for (State t : currentState.transition(l)) {
                CreateSVPG cs = new CreateSVPG(sVPG, fts, t, unfoldingMapping);
                formula.getFormula().accept(cs);

                Vertex tv = sVPG.findVertex(t, formula.getFormula());

                Edge e = new Edge();
                e.configurations = l.getFeatureExpression();
                e.target = tv;

                v.addEdge(e);
            }
        }
    }

    @Override
    public void visit(DiamondFormula formula) {
        Vertex v = createVertexIfItDoesntExist(formula);
        if (v == null)
            return;
        v.owner = 0;
        v.prio = 0;

        for (Label l : currentState.getTransitionLabels()) {
            List<String> actions = Arrays.asList(formula.getAction().getText().split("\\|\\|"));
            if (!actions.contains(l.getText()))
                continue;
            for (State t : currentState.transition(l)) {
                CreateSVPG cs = new CreateSVPG(sVPG, fts, t, unfoldingMapping);
                formula.getFormula().accept(cs);

                Vertex tv = sVPG.findVertex(t, formula.getFormula());

                Edge e = new Edge();
                e.configurations = l.getFeatureExpression();
                e.target = tv;

                v.addEdge(e);
            }
        }
    }

    @Override
    public void visit(LiteralFalse formula) {
        Vertex v = createVertexIfItDoesntExist(formula);
        if (v == null)
            return;
        v.owner = 0;
        v.prio = 0;
        super.visit(formula);
    }

    @Override
    public void visit(LiteralTrue formula) {
        Vertex v = createVertexIfItDoesntExist(formula);
        if (v == null)
            return;
        v.owner = 1;
        v.prio = 0;
        super.visit(formula);
    }

    @Override
    public void visit(LogicAndFormula formula) {
        Vertex v = createVertexIfItDoesntExist(formula);
        if (v == null)
            return;
        v.owner = 1;
        v.prio = 0;
        super.visit(formula);
        Vertex tvl = sVPG.findVertex(currentState, formula.getLeft());
        Vertex tvr = sVPG.findVertex(currentState, formula.getRight());
        Edge el = new Edge();
        el.target = tvl;
        el.configurations = FeatureDiagram.PrimaryFD.factory.one();

        Edge er = new Edge();
        er.target = tvr;
        er.configurations = FeatureDiagram.PrimaryFD.factory.one();

        v.addEdge(el);
        v.addEdge(er);
    }

    @Override
    public void visit(LogicOrFormula formula) {
        Vertex v = createVertexIfItDoesntExist(formula);
        if (v == null)
            return;
        v.owner = 0;
        v.prio = 0;
        super.visit(formula);
        Vertex tvl = sVPG.findVertex(currentState, formula.getLeft());
        Vertex tvr = sVPG.findVertex(currentState, formula.getRight());
        Edge el = new Edge();
        el.target = tvl;
        el.configurations = FeatureDiagram.PrimaryFD.factory.one();

        Edge er = new Edge();
        er.target = tvr;
        er.configurations = FeatureDiagram.PrimaryFD.factory.one();

        v.addEdge(el);
        v.addEdge(er);
    }

    @Override
    public void visit(MuFormula formula) {
        this.unfoldingMapping.put(formula.getRecursionVariable(), formula);
        Vertex v = createVertexIfItDoesntExist(formula);
        if (v == null)
            return;
        v.owner = 0;
        v.prio = ((int) formula.getAlternationDepth() / 2) * 2 + 1;

        super.visit(formula);

        Vertex tv = sVPG.findVertex(currentState, formula.getFormula());
        Edge e = new Edge();
        e.target = tv;
        e.configurations = FeatureDiagram.PrimaryFD.factory.one();

        v.addEdge(e);
    }

    @Override
    public void visit(NuFormula formula) {
        this.unfoldingMapping.put(formula.getRecursionVariable(), formula);
        Vertex v = createVertexIfItDoesntExist(formula);
        if (v == null)
            return;
        v.owner = 0;
        v.prio = ((int) formula.getAlternationDepth() / 2) * 2;

        super.visit(formula);

        Vertex tv = sVPG.findVertex(currentState, formula.getFormula());
        Edge e = new Edge();
        e.target = tv;
        e.configurations = FeatureDiagram.PrimaryFD.factory.one();

        v.addEdge(e);
    }

    @Override
    public void visit(RecursionVariable formula) {
        Vertex v = createVertexIfItDoesntExist(formula);
        if (v == null)
            return;
        v.owner = 0;
        v.prio = 0;

        Formula originFormula = unfoldingMapping.get(formula);

        Vertex vt;
        vt = sVPG.findVertex(currentState, originFormula);
        if (vt == null) {
            CreateSVPG cs = new CreateSVPG(sVPG, fts, currentState, unfoldingMapping);
            originFormula.accept(cs);
            vt = sVPG.findVertex(currentState, originFormula);
        }
        Edge e = new Edge();
        e.configurations = FeatureDiagram.PrimaryFD.factory.one();
        e.target = vt;
        v.addEdge(e);
    }

    private Vertex createVertexIfItDoesntExist(Formula formula) {
        Vertex v = sVPG.findVertex(currentState, formula);
        if (v == null) {
            v = sVPG.addVertex(currentState, formula);
            return v;
        }
        return null;
    }
}
