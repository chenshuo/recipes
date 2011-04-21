package billing;

public class DefaultRule extends Rule {

    private final String name;

    public DefaultRule(String name) {
        this.name = name;
    }

    @Override
    public long getMoneyInPips(UserMonthUsage input) {
        throw new RuntimeException("Rule " + name + " not implemented");
    }

}
