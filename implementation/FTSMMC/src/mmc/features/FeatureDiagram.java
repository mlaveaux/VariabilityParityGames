package mmc.features;

import  net.sf.javabdd.*;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Random;

public class FeatureDiagram {
    public BDDFactory factory;
    public static FeatureDiagram PrimaryFD;
    private String[] features;
    public BDD FD;
    public List<BDD> products;
    public List<String> productStrings;

    public List<BDD> collectionItems;

    public int varCount()
    {
        return factory.varNum();
    }


    public static FeatureDiagram FeatureDiagramFromBDD(String[] features, String BDD) throws Exception {
        FeatureDiagram fd = new FeatureDiagram(features);
        fd.FD = BDDParser.parseBDD(BDD, fd);
        fd.iniate();
        return fd;
    }

    private FeatureDiagram(String[] features) {
        factory = BDDFactory.init("j",1000000,100000);
        this.features = features;
        factory.setVarNum(this.features.length);
    }

    protected void iniate(){
        products = new ArrayList<>();
        productStrings = new ArrayList<>();
        findValidProducts(factory.one(),0, "");
        buildSet();
    }

    protected void buildSet(){
//        factory.makeSet()
        int nrofconfs = (1 << varCount());
        collectionItems = new ArrayList<>();
        for(int i = 0;i<nrofconfs;i++) {
            BDD confbdd = factory.one();
            int conf = i;
            for (int j = varCount() - 1; j >= 0; j--) {
                if ((1 << j) <= conf) {
                    conf -= (1 << j);
                    BDD a = factory.ithVar(j);
                    confbdd.andWith(a.id());
//                    confbdd = super.and(confbdd, this.featureVariables[j]);
                } else {
                    BDD a = factory.ithVar(j);
                    confbdd.andWith(a.id().not().id());
                }
            }
            collectionItems.add(confbdd);
        }
    }

    private void findValidProducts(BDD i, int f, String productString)
    {
        if(f >= features.length)
        {
            if(!i.and(FD.id()).isZero()){
                products.add(i);
                productStrings.add(productString);
            }
        } else {
            findValidProducts(i.and(factory.ithVar(f).id()), f + 1, productString + '1');
            findValidProducts(i.and(factory.ithVar(f).id().not()), f + 1, productString + '0');
        }
    }

    public BDD getVariable(String feature)
    {
        for(int i = 0;i<features.length;i++)
        {
            if(feature.compareTo(features[i]) == 0)
                return factory.ithVar(i);
        }
        return null;
    }

    public BDD getFeaturedRandomConfigurations(float lambda){
        double cutoff = (Math.log(lambda)/Math.log(2) + features.length) / features.length;
        BDD result = this.FD.id();
        Random r = new Random();
        for(int i = 0;i<varCount();i++){
            float f = r.nextFloat();
            if(f >= cutoff){
                if(f >= lambda + (1-cutoff)/2){
                    result.andWith(factory.ithVar(i).id());
                } else {
                    result.andWith(factory.ithVar(i).id().not().id());
                }
            }
        }
        return result;
    }

    public BDD getRandomConfigurations(float portion){
        BDD result = factory.zero();
        int nrofconfs = (1 << varCount());
        Collections.shuffle(collectionItems);
        nrofconfs = (int)((float)nrofconfs * portion);
        nrofconfs = Math.max(nrofconfs, 1);
        for(int i = 0;i<nrofconfs;i++){
            result.orWith(collectionItems.get(i).id());
        }
        return result;
    }
}
