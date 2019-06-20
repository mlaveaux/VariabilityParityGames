package mmc.SVPG;

import javafx.util.Pair;
import mmc.features.FeatureDiagram;
import mmc.modal.formulas.Formula;
import mmc.models.State;

import javax.management.Query;
import java.awt.*;
import java.io.*;
import java.util.*;
import java.util.List;

public class SVPG {
    private Vertex[] vertices;
    int n;
    private HashMap<State, HashMap<Formula, Vertex>> mapje;

    private SVPG()
    {
        mapje = new HashMap<>();
    }
    public SVPG(Vertex[] vertices){
        this();
        n = vertices.length;
        this.vertices = new Vertex[n + 2];
        for(int i = 0;i<n;i++)
            this.vertices[i] = vertices[i];
        this.vertices[n] = new Vertex();
        this.vertices[n+1] = new Vertex();
    }

    public SVPG(int n) {
        this();
        this.vertices = new Vertex[n];
        for(int i = 0;i<n;i++)
            this.vertices[i] = new Vertex();
        this.n = 0;
    }

    public Vertex findVertex(State originState, Formula originFormula)
    {
        System.out.println("Finding, length: " + String.valueOf(n));
        long start = System.nanoTime();
//        for(Vertex v : vertices)
//        {
//            if(v.originFormula.equals(originFormula) && v.originState.sameStateNr(originState)){
//
//                System.out.println("Took: " + String.valueOf(start - System.nanoTime()) + " ns");
//                return v;
//            }
//        }

        HashMap<Formula, Vertex> a  = mapje.get(originState);
        if(a != null){
            System.out.println("Took: " + String.valueOf(start - System.nanoTime()) + " ns");
            return a.get(originFormula);
        }
        System.out.println("Took: " + String.valueOf(start - System.nanoTime()) + " ns");
        return null;
    }

    public Vertex addVertex(State originState, Formula originFormula)
    {
        Vertex v = vertices[n++];
        v.originState = originState;
        v.originFormula = originFormula;

        HashMap<Formula, Vertex> a  = mapje.get(originState);
        if(a == null)
        {
            a =  new HashMap();
            mapje.put(originState, a);
        }
        a.put(originFormula, v);
        return v;
    }

    public void makeInfinite(){
        Vertex l0 = vertices[n++];
        l0.owner = 0;
        l0.prio = 1;

        Edge e0 = new Edge();
        e0.configurations = 1;
        e0.target = l0;
        l0.addEdge(e0);

        Vertex l1 = vertices[n++];
        l1.owner = 0;
        l1.prio = 0;

        Edge e1 = new Edge();
        e1.configurations = 1;
        e1.target = l1;
        l1.addEdge(e1);

        for(int i = 0;i<n;i++)
        {
            Vertex v = this.vertices[i];
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
        for(int i = 0;i<n;i++)
        {
            Vertex v = this.vertices[i];
            Index.put(v, i);
        }
        StringBuilder sb = new StringBuilder();
        sb.append("parity " + String.valueOf(n) + ";\n");

        for(int i = 0;i<n;i++)
        {
            Vertex v = this.vertices[i];
            sb.append(String.format("%d %d %d ",Index.get(v), v.prio, v.owner));
            List<String> targets = new ArrayList<>();
            for(Edge e : v.edges){
                if(FeatureDiagram.PrimaryFD.and(e.configurations, product) > 0)
                    targets.add(String.valueOf(Index.get(e.target)));
            }

            sb.append(String.join(",", targets));
            sb.append(";\n");
        }
        return sb.toString().trim();
    }

    public String toSVPG() throws UnsupportedEncodingException {
        Map<Vertex, Integer> Index = new HashMap<>();
        for(int i = 0;i<n;i++)
        {
            Vertex v = this.vertices[i];
            Index.put(v, 0000+(i));
        }

        StringBuilder sb = new StringBuilder();
        String boolLiteral = "";
        for(int i = 0;i<FeatureDiagram.PrimaryFD.varCount();i++)
            boolLiteral+="-";
        sb.append("confs ");
        sb.append(bddIntToString(FeatureDiagram.PrimaryFD.FD,boolLiteral));

        sb.append(";\nparity " + String.valueOf(n) + ";\n");
        for(int i = 0;i<n;i++)
        {
            Vertex v = this.vertices[i];
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
