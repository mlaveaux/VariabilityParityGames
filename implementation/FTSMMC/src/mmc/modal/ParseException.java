package mmc.modal;

public class ParseException extends Exception {
    public ParseException(String message, int index)
    {
        super("Parse error at " + String.valueOf(index) + ".\n" + message);
    }
}
