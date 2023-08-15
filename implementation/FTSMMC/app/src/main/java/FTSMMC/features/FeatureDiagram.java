package FTSMMC.features;

import net.sf.javabdd.*;

import java.util.ArrayList;
import java.util.List;

public class FeatureDiagram {
    public BDDFactory factory;
    public static FeatureDiagram PrimaryFD;
    private String[] features;
    public BDD FD;
    public List<BDD> products;
    public List<String> productStrings;

    public int varCount() {
        return factory.varNum();
    }

    public static FeatureDiagram FeatureDiagramFromBDD(String[] features, String BDD) throws Exception {
        FeatureDiagram fd = new FeatureDiagram(features);
        fd.FD = BDDParser.parseBDD(BDD, fd);
        fd.iniate();
        return fd;
    }

    private FeatureDiagram(String[] features) {
        factory = BDDFactory.init("j", 1000000, 100000);
        this.features = features;
        factory.setVarNum(this.features.length);
    }

    protected void iniate() {
        products = new ArrayList<>();
        productStrings = new ArrayList<>();
        findValidProducts(factory.one(), 0, "");
    }

    private void findValidProducts(BDD i, int f, String productString) {
        if (f >= features.length) {
            if (!i.and(FD.id()).isZero()) {
                products.add(i);
                productStrings.add(productString);
            }
        } else {
            findValidProducts(i.and(factory.ithVar(f).id()), f + 1, productString + '1');
            findValidProducts(i.and(factory.ithVar(f).id().not()), f + 1, productString + '0');
        }
    }

    public BDD getVariable(String feature) throws Exception {
        for (int i = 0; i < features.length; i++) {
            if (feature.compareTo(features[i]) == 0)
                return factory.ithVar(i);
        }

        throw new Exception("Could not find variable " + feature + " in " + features);
    }
}
