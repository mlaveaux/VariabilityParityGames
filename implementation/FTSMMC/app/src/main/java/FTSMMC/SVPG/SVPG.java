package FTSMMC.SVPG;

import FTSMMC.features.FeatureDiagram;
import FTSMMC.modal.formulas.Formula;
import FTSMMC.models.State;
import net.sf.javabdd.BDD;

import java.io.*;
import java.util.*;
import java.util.List;

public class SVPG {
    private Vertex[] vertices;
    int n;
    private HashMap<State, HashMap<Formula, Vertex>> mapje;

    private SVPG() {
        mapje = new HashMap<>();
    }

    public SVPG(Vertex[] vertices) {
        this();
        n = vertices.length;
        this.vertices = new Vertex[n + 2];
        for (int i = 0; i < n; i++)
            this.vertices[i] = vertices[i];
        this.vertices[n] = new Vertex();
        this.vertices[n + 1] = new Vertex();
    }

    public SVPG(int n) {
        this();
        this.vertices = new Vertex[n];
        for (int i = 0; i < n; i++)
            this.vertices[i] = new Vertex();
        this.n = 0;
    }

    public Vertex findVertex(State originState, Formula originFormula) {
        // System.out.println("Finding, length: " + String.valueOf(n));
        // long start = System.nanoTime();
        // for(Vertex v : vertices)
        // {
        // if(v.originFormula.equals(originFormula) &&
        // v.originState.sameStateNr(originState)){
        //
        // System.out.println("Took: " + String.valueOf(start - System.nanoTime()) + "
        // ns");
        // return v;
        // }
        // }

        HashMap<Formula, Vertex> a = mapje.get(originState);
        if (a != null) {
            // System.out.println("Took: " + String.valueOf(System.nanoTime() - start) + "
            // ns");
            return a.get(originFormula);
        }
        // System.out.println("Took: " + String.valueOf(System.nanoTime() - start) + "
        // ns");
        return null;
    }

    public Vertex addVertex(State originState, Formula originFormula) {
        Vertex v = vertices[n++];
        v.originState = originState;
        v.originFormula = originFormula;

        HashMap<Formula, Vertex> a = mapje.get(originState);
        if (a == null) {
            a = new HashMap<Formula, Vertex>();
            mapje.put(originState, a);
        }
        a.put(originFormula, v);
        return v;
    }

    public void makeInfinite() {
        Vertex l0 = vertices[n++];
        l0.owner = 0;
        l0.prio = 1;

        Edge e0 = new Edge();
        e0.configurations = FeatureDiagram.PrimaryFD.factory.one();
        e0.target = l0;
        l0.addEdge(e0);

        Vertex l1 = vertices[n++];
        l1.owner = 0;
        l1.prio = 0;

        Edge e1 = new Edge();
        e1.configurations = FeatureDiagram.PrimaryFD.factory.one();
        e1.target = l1;
        l1.addEdge(e1);

        for (int i = 0; i < n; i++) {
            Vertex v = this.vertices[i];
            BDD edgeDisjunction = FeatureDiagram.PrimaryFD.factory.zero();
            for (Edge e : v.edges)
                edgeDisjunction.orWith(e.configurations.id());
            if (!edgeDisjunction.isOne()) {
                // System.err.println("This happens: " + String.valueOf(i));
                Edge sinkEdge = new Edge();
                sinkEdge.target = (v.owner == 0) ? l0 : l1;
                sinkEdge.configurations = edgeDisjunction.id().not();
                sinkEdge.configurations.andWith(FeatureDiagram.PrimaryFD.FD.id());
                v.addEdge(sinkEdge);
                sinkEdge = null;
            }
        }
    }

    public String projectToPG(BDD product) {
        Map<Vertex, Integer> Index = new HashMap<>();
        for (int i = 0; i < n; i++) {
            Vertex v = this.vertices[i];
            Index.put(v, i);
        }
        StringBuilder sb = new StringBuilder();
        sb.append("parity " + String.valueOf(n) + ";\n");

        for (int i = 0; i < n; i++) {
            Vertex v = this.vertices[i];
            sb.append(String.format("%d %d %d ", Index.get(v), v.prio, v.owner));
            List<String> targets = new ArrayList<>();
            for (Edge e : v.edges) {
                if (!e.configurations.and(product).isZero())
                    targets.add(String.valueOf(Index.get(e.target)));
            }

            sb.append(String.join(",", targets));
            sb.append(";\n");
        }
        return sb.toString().trim();
    }

    public String toSVPG() throws UnsupportedEncodingException {
        Map<Vertex, Integer> Index = new HashMap<>();
        for (int i = 0; i < n; i++) {
            Vertex v = this.vertices[i];
            Index.put(v, 0000 + (i));
        }

        StringBuilder sb = new StringBuilder();
        String boolLiteral = "";
        for (int i = 0; i < FeatureDiagram.PrimaryFD.varCount(); i++)
            boolLiteral += "-";
        sb.append("confs ");
        sb.append(bddIntToString(FeatureDiagram.PrimaryFD.FD, boolLiteral));

        sb.append(";\nparity " + String.valueOf(n) + ";\n");
        for (int i = 0; i < n; i++) {
            Vertex v = this.vertices[i];
            sb.append(String.format("%d %d %d ", Index.get(v), v.prio, v.owner));
            List<String> targets = new ArrayList<>();
            BDD conf = FeatureDiagram.PrimaryFD.factory.zero();
            for (Edge e : v.edges) {
                targets.add(
                        String.valueOf(Index.get(e.target)) + "|" + bddIntToString(e.configurations.id(), boolLiteral));
                conf.orWith(e.configurations.id());
            }
            if (!conf.isOne()) {
                // TODO: I don't know if this is an error, but printing is slow.
                // System.err.println("Incomplete: " + String.valueOf(i));
                for (Edge e : v.edges) {
                    bddIntToString(e.configurations, boolLiteral);
                }
            }

            sb.append(String.join(",", targets));
            sb.append(";\n");
        }
        return sb.toString().trim();
    }

    private String bddIntToString(BDD bdd, String trueLiteral) throws UnsupportedEncodingException {
        ByteArrayOutputStream baos = new ByteArrayOutputStream();
        PrintStream ps = new PrintStream(baos);
        System.setOut(ps);

        List l = bdd.allsat();
        String sep = "";
        for (Object o : l) {
            System.out.print(sep);
            byte[] b = (byte[]) o;
            for (int i = 0; i < FeatureDiagram.PrimaryFD.varCount(); i++) {
                if (b[i] == -1)
                    System.out.print('-');
                else
                    System.out.print(b[i]);
            }
            sep = "+";
        }
        // FeatureDiagram.PrimaryFD.printSet(bdd);
        ps.flush();
        byte[] products = baos.toByteArray();
        String s = new String(products, "UTF-8");
        s = s.trim().replace('\n', '+');
        System.setOut(new PrintStream(new FileOutputStream(FileDescriptor.out)));
        s = s.replace("TRUE", trueLiteral);
        s = s.replace("FALSE", "F");
        return s;
    }
}
