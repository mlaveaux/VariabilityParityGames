package mmc.features;

import jdd.bdd.BDD;

import java.util.ArrayList;
import java.util.List;
import java.util.Random;

public class FeatureDiagram extends BDD {
    public static FeatureDiagram PrimaryFD;
    private String[] features;
    private int[] featureVariables;
    public int FD;
    public List<Integer> products;
    public List<String> productStrings;

    public int varCount()
    {
        return featureVariables.length;
    }


    public static FeatureDiagram FeatureDiagramFromBDD(String[] features, String BDD) throws Exception {
        FeatureDiagram fd = new FeatureDiagram(features);
        fd.FD = BDDParser.parseBDD(BDD, fd);
        fd.iniate();
        return fd;
    }

    private FeatureDiagram(String[] features) {
        super(1000,1000);
        this.features = features;
        this.featureVariables = new int[this.features.length];
        for(int i = 0;i<this.features.length;i++)
        {
            this.featureVariables[i] = super.createVar();
        }
    }

    protected void iniate(){
        products = new ArrayList<>();
        productStrings = new ArrayList<>();
        findValidProducts(1,0, "");
    }

    private void findValidProducts(int i, int f, String productString)
    {
        if(f >= features.length)
        {
            if(super.and(i, FD) > 0) {
                products.add(i);
                productStrings.add(productString);
            }
        } else {
            findValidProducts(super.and(i, featureVariables[f]), f + 1, productString + '1');
            findValidProducts(super.and(i, super.not(featureVariables[f])), f + 1, productString + '0');
        }
    }

    public int getVariable(String feature)
    {
        for(int i = 0;i<features.length;i++)
        {
            if(feature.compareTo(features[i]) == 0)
                return featureVariables[i];
        }
        return -1;
    }

    public int getRandomConfigurations(float lambda){
        int result = this.FD;
        Random r = new Random();
        for(int i = 0;i<featureVariables.length;i++){
            float f = r.nextFloat();
            if(f >= lambda){
                int v = getVariable(String.valueOf(i));
                if(f >= lambda + (1-lambda)/2){
                    result = super.and(result,v);
                } else {
                    result = super.and(result, super.not(v));
                }
            }
        }
        return result;
    }

    public int getRandomConfigurations(){
        int result = super.getZero();
        Random r = new Random();
        int nrofconfs = 1+r.nextInt((1 << this.num_vars)-1);
        for(int i = 0;i<nrofconfs;i++){
            int conf = r.nextInt(1 << this.num_vars);
            int confbdd = super.getOne();
            for(int j = 0;j< this.num_vars;j++){
                if((1 << j) <= conf){
                    conf -= (1<<j);
                    confbdd = super.and(confbdd,this.featureVariables[j]);
                } else {
                    confbdd = super.and(confbdd,super.not(this.featureVariables[j]));
                }
            }
            result = super.or(result,confbdd);
        }
        return result;
    }
}
