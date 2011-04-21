package billing;

public class BillCalculator {
    private final RuleFactory factory;

    public BillCalculator(String rulePath) {
        factory = new RuleFactory(rulePath);
    }

    public Money calculate(UserMonthUsage input) {
        Rule rule = factory.create("root");
        return new Money(rule.getMoneyInPips(input));
    }
}
