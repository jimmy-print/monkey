void pfa(std::vector<float> v)
{
    for (std::vector<float>::const_iterator i = v.begin(); i != v.end(); i++)
        std::cout << *i << " ";
    std::cout << "\n";
}
