package mmc.SVPG;

import mmc.features.FeatureDiagram;
import mmc.modal.formulas.Formula;
import mmc.models.State;

import java.awt.*;
import java.io.*;
import java.util.*;
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
            int edgeDisjunction = 0;
            for(Edge e : v.edges)
                edgeDisjunction = FeatureDiagram.PrimaryFD.or(edgeDisjunction, e.configurations);
            if(edgeDisjunction != 1){
                Edge sinkEdge = new Edge();
                sinkEdge.target = (v.owner == 0)?l0:l1;
                sinkEdge.configurations = FeatureDiagram.PrimaryFD.not(edgeDisjunction);
                v.addEdge(sinkEdge);
                sinkEdge = null;
            }
        }
    }

    public String projectToPG(int product)
    {
        Map<Vertex, Integer> Index = new HashMap<>();
        int i = 0;
        for(Vertex v : vertices){
            Index.put(v, i++);
        }
        StringBuilder sb = new StringBuilder();
        sb.append("parity " + String.valueOf(vertices.size()) + ";\n");
        for(Vertex v : vertices){
            sb.append(String.format("%d %d %d ",Index.get(v), v.prio, v.owner));
            List<String> targets = new ArrayList<>();
            for(Edge e : v.edges){
                if(FeatureDiagram.PrimaryFD.and(e.configurations, product) > 0)
                    targets.add(String.valueOf(Index.get(e.target)));
            }

            sb.append(String.join(",", targets));
            sb.append(";\n");
        }
        return sb.toString();
    }

    public String toSVPG() throws UnsupportedEncodingException {
        Map<Vertex, Integer> Index = new HashMap<>();
        int i = 0;
        for(Vertex v : vertices){
            Index.put(v, i++);
        }

        StringBuilder sb = new StringBuilder();
        String boolLiteral = "";
        for(i = 0;i<FeatureDiagram.PrimaryFD.varCount();i++)
            boolLiteral+="-";
        sb.append("confs ");
        sb.append(bddIntToString(FeatureDiagram.PrimaryFD.FD,boolLiteral));

        sb.append(";\nparity " + String.valueOf(vertices.size()) + ";\n");
        for(Vertex v : vertices){
            sb.append(String.format("%d %d %d ",Index.get(v), v.prio, v.owner));
            List<String> targets = new ArrayList<>();
            for(Edge e : v.edges){
                    targets.add(String.valueOf(Index.get(e.target)) + "|" + bddIntToString(e.configurations,boolLiteral));
            }

            sb.append(String.join(",", targets));
            sb.append(";\n");
        }
        return sb.toString().trim();
    }

    private String bddIntToString(int bdd, String trueLiteral) throws UnsupportedEncodingException {
        ByteArrayOutputStream baos = new ByteArrayOutputStream();
        PrintStream ps = new PrintStream(baos);
        System.setOut(ps);

        FeatureDiagram.PrimaryFD.printSet(bdd);
        ps.flush();
        byte[] products = baos.toByteArray();
        String s = new String(products, "UTF-8");
        s = s.trim().replace('\n','+');
        System.setOut(new PrintStream(new FileOutputStream(FileDescriptor.out)));
        s = s.replace("TRUE",trueLiteral);
        return s;
    }
}
